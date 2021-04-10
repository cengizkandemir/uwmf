#include <chrono>
#include <exception>
#include <ostream>
#include <optional>
#include <string>
#include <sys/types.h>

#include "image.h"
#include "image_utils.h"
#include "logger.h"
#include "math_utils.h"
#include "png_image.h"
#include "uwmf.h"

#include "../external/cxxopts/include/cxxopts.hpp"

namespace
{

enum class mode
{
    RESTORATION,
    CORRUPTION,
    SIMULATION
};

struct program_options
{
    mode m;        // mode
    int k;         // weight fall-off
    int p;         // Minkowski exponent
    int w;         // filtering window size
    double d;      // corruption density
    int r;         // repeat counter
    std::string i; // input image
    std::string o; // output image
};

std::optional<mode> to_mode(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
            [] (unsigned char ch) { return std::tolower(ch); });

    if(str == "r" || str == "restoration") {
        return mode::RESTORATION;
    }
    else if(str == "c" || str == "corruption") {
        return mode::CORRUPTION;
    }
    else if(str == "s" || str == "simulation") {
        return mode::SIMULATION;
    }

    return std::nullopt;
}

std::string to_string(mode m)
{
    switch(m) {
    case mode::RESTORATION: return "restoration"; break;
    case mode::CORRUPTION: return "corruption"; break;
    case mode::SIMULATION: return "simulation"; break;
    default: ASSERT(false, "invalid mode"); break;
    }

    return "";
}

std::ostream& operator<<(std::ostream& out, const program_options& opts)
{
    out << "\n";
    out << "    m = " << to_string(opts.m) << "\n";
    if(opts.m == mode::SIMULATION) {
        out << "    r = " << opts.r << "\n";
    }

    if(opts.m == mode::RESTORATION || opts.m == mode::SIMULATION) {
        out << "    k = " << opts.k << "\n";
        out << "    p = " << opts.p << "\n";
        out << "    w = " << opts.w << "\n";
    }

    if(opts.m == mode::CORRUPTION || opts.m == mode::SIMULATION) {
        out << "    d = " << opts.d << "\n";
    }

    out << "    i = " << opts.i << "\n";

    if(opts.m != mode::SIMULATION) {
        out << "    o = " << opts.o;
    }

    return out;
}

constexpr const char* help()
{
    return "uwmf [-m r] -i <...> -w <...> [-k <...>] [-p <...>] [-o <...>]\n  "
            "uwmf -m c -i <...> -d <...> [-o <...>]\n  "
            "uwmf -m s -i <...> -w <...> -d <...> [-k <...>] [-p <...>]"
                "[-r <...>]";

}

std::optional<program_options> extract_program_options(
        const cxxopts::ParseResult& results)
{
    program_options opts{};

    auto m = to_mode(results["m"].as<std::string>());
    if(!m) {
        LOGE() << "unrecognized mode";
        return std::nullopt;
    }
    opts.m = *m;

    if(results["i"].count() == 0) {
        LOGE() << "missing input";
        return std::nullopt;
    }
    opts.i = results["i"].as<std::string>();

    if(*m != mode::SIMULATION) {
        if(results["o"].count() == 0) {
            opts.o = opts.i;
            std::size_t dot_pos = opts.o.find_last_of('.');
            if(dot_pos == std::string::npos) {
                dot_pos = opts.o.length();
            }
            opts.o.insert(dot_pos,
                    opts.m == mode::RESTORATION ? "_restored" : "_corrupted");
        }
        else {
            opts.o = results["o"].as<std::string>();
        }
    }
    else {
        if(results["r"].count() == 0) {
            LOGE() << "missing option r";
            return std::nullopt;
        }

        opts.r = results["r"].as<int>();
    }

    if(*m == mode::RESTORATION || *m == mode::SIMULATION) {
        if(results["w"].count() == 0) {
            LOGE() << "missing option w";
            return std::nullopt;
        }

        opts.w = results["w"].as<int>();
        opts.k = results["k"].as<int>();
        opts.p = results["p"].as<int>();
    }

    if(*m == mode::CORRUPTION || *m == mode::SIMULATION) {
        if(results["d"].count() == 0) {
            LOGE() << "missing option d";
            return std::nullopt;
        }

        opts.d = results["d"].as<double>();
    }

    return opts;
}

} // anonymous

int main(int argc, char** argv)
{
    uwmf::logger::filter() = uwmf::logger::log_level::INFO;

    cxxopts::Options opts("uwmf");
    opts.custom_help(help());

    opts.add_options()
            (
                    "h,help",
                    "Display help"
            )
            (
                    "m,mode",
                    "Program mode",
                    cxxopts::value<std::string>()->default_value("r")
            )
            (
                    "k,weight-fall-off",
                    "Weight fall-off",
                    cxxopts::value<int>()->default_value("4")
            )
            (
                    "p,minkowski-exponent",
                    "Minkowski exponent",
                    cxxopts::value<int>()->default_value("1")
            )
            (
                    "w,filtering-window-size",
                    "Filtering window size",
                    cxxopts::value<int>()
            )
            (
                    "d,corruption-density",
                    "Image corruption density ()",
                    cxxopts::value<double>()
            )
            (
                    "r,repeat",
                    "Metrics calculation repeat counter",
                    cxxopts::value<int>()->default_value("1")
            )
            (
                    "i,input",
                    "Input file name",
                    cxxopts::value<std::string>()
            )
            (
                    "o,output",
                    "Output file name",
                    cxxopts::value<std::string>()
            );



    program_options optvals{};
    try {
        cxxopts::ParseResult parse_results = opts.parse(argc, argv);

        if(parse_results.count("help")
                || parse_results.arguments().size() == 0) {
            LOGI() << opts.help();
            return 0;
        }

        auto result = extract_program_options(parse_results);
        if(!result) {
            return -1;
        }
        optvals = *result;

    }
    catch(const std::exception& e) {
        LOGE() << e.what();
        return -1;
    }

    LOGD() << "running UWMF with" << optvals;

    uwmf::monochrome_png_image png = *uwmf::read_png_image(optvals.i);
    uwmf::monochrome_image input_image(png.buffer, png.width, png.height);

    if(optvals.m == mode::CORRUPTION) {
        auto corrupt_image = fvin(input_image, optvals.d);
        uwmf::write_png_image(corrupt_image.data(),
                corrupt_image.width(), corrupt_image.height(), optvals.o);
    }
    else if(optvals.m == mode::RESTORATION) {
        auto restored_image = uwmf::uwmf(input_image,
                uwmf::naive_noise_detector, {optvals.w, optvals.p, optvals.k});
        uwmf::write_png_image(restored_image.data(),
                restored_image.width(), restored_image.height(), optvals.o);
    }
    else {
        double psnr_sum = 0;
        double ssim_sum = 0;
        double ief_sum = 0;
        long int time_sum = 0;

        for(int i = 0; i < optvals.r; i++) {
            auto corrupt_image = fvin(input_image, optvals.d);

            auto t1 = std::chrono::high_resolution_clock::now();
            auto restored_image = uwmf::uwmf(corrupt_image,
                    uwmf::naive_noise_detector,
                    {optvals.w, optvals.p, optvals.k});
            auto t2 = std::chrono::high_resolution_clock::now();

            psnr_sum += uwmf::psnr(input_image, restored_image);
            ssim_sum += uwmf::ssim(input_image, restored_image);
            ief_sum += uwmf::ief(input_image, restored_image, corrupt_image);
            time_sum += std::chrono::duration_cast<
                    std::chrono::milliseconds>(t2 - t1).count();
        }

        LOGI() << "corruption density    : " << optvals.d;
        LOGI() << "average psnr          : " << psnr_sum / optvals.r;
        LOGI() << "average ssim          : " << ssim_sum / optvals.r;
        LOGI() << "average ief           : " << ief_sum / optvals.r;
        LOGI() << "average cpu time (ms) : " << time_sum / optvals.r;
    }

    return 0;
}
