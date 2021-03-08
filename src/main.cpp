#include <array>
#include <charconv>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "image.h"
#include "image_utils.h"
#include "logger.h"
#include "math_utils.h"
#include "png_image.h"

namespace
{

constexpr int DEFAULT_WEIGHT_FALL_OFF = 4;
constexpr int DEFAULT_MINKOWSKI_EXPONENT = 1;

struct program_options
{
    int k = DEFAULT_WEIGHT_FALL_OFF;     // weight fall-off
    int p = DEFAULT_MINKOWSKI_EXPONENT;  // Minkowski exponent
    int w;         // filtering window size
    std::string i; // input image
    std::string o; // output image
};

std::ostream& operator<<(std::ostream& out, const program_options& opts)
{
    out << "\n";
    out << "    k = " << opts.k << "\n";
    out << "    p = " << opts.p << "\n";
    out << "    w = " << opts.w << "\n";
    out << "    i = " << opts.i << "\n";
    out << "    o = " << opts.o;

    return out;
}

std::string help()
{
    std::stringstream ss;
    ss << "options:\n"
            << "    -k: weight fall-off (default: "
            << DEFAULT_WEIGHT_FALL_OFF << ")\n"
            << "    -p: Minkowski exponent (default: "
            << DEFAULT_MINKOWSKI_EXPONENT << ")\n"
            << "    -w: filtering window size\n"
            << "    -i: input png image\n"
            << "    -o: optional output file\n"
            << "        if omitted, input file name with \"_restored\" "
                       "suffix will be used";

    return ss.str();
}

std::optional<std::string> extract_opt(const std::string& arg,
        std::size_t eqsgn_pos)
{
    if(eqsgn_pos == 0) {
        LOGE() << "expected an option before =";
        return std::nullopt;
    }

    return arg.substr(0, eqsgn_pos);
}

std::optional<std::string> extract_val(const std::string& arg,
        std::size_t eqsgn_pos)
{
    if(eqsgn_pos + 1 ==  arg.length()) {
        LOGE() << "exepcted a value after =";
        return std::nullopt;
    }

    return arg.substr(eqsgn_pos + 1);
}

std::optional<std::pair<std::string, std::string>> extract_opt_and_val(
        const std::string& arg, std::size_t eqsgn_pos)
{
    auto opt = extract_opt(arg, eqsgn_pos);
    auto val = extract_val(arg, eqsgn_pos);

    if(!opt || !val) {
        return std::nullopt;
    }

    return std::make_pair(opt.value(), val.value());
}

std::optional<int> convert(const std::string& str)
{
    int result;
    auto [match, err] = std::from_chars(
            str.data(), str.data() + str.size(), result);

    if(err != std::errc()) {
        return std::nullopt;
    }

    return result;
}

std::optional<program_options> convert_opts(
        const std::unordered_map<std::string, std::string>& opts_map)
{
    program_options opts;

    if(opts_map.count("k")) {
        auto k = convert(opts_map.at("k"));
        if(!k) {
            LOGE() << "failed to convert value for option k";
            return std::nullopt;
        }
        opts.k = k.value();
    }

    if(opts_map.count("p")) {
        auto p = convert(opts_map.at("p"));
        if(!p) {
            LOGE() << "failed to convert value for option p";
            return std::nullopt;
        }
        opts.p = p.value();
    }

    ASSERT(opts_map.count("w"), "missing non optional option");
    auto w = convert(opts_map.at("w"));
    if(!w) {
        LOGE() << "failed to convert value for option w";
        return std::nullopt;
    }
    opts.w = w.value();

    ASSERT(opts_map.count("i"), "missing non optional option");
    opts.i = opts_map.at("i");

    if(opts_map.count("o")) {
        opts.o = opts_map.at("o");
    }
    else {
        opts.o = opts.i;
        std::size_t dot_pos = opts.o.find('.');
        if(dot_pos == std::string::npos) {
            LOGE() << "missing file extension";
            return std::nullopt;
        }
        opts.o.insert(dot_pos, "_restored");
    }

    return opts;
}

std::optional<program_options> parse_program_opts(int argc, char** argv)
{
    bool expected_val = false;
    std::string last_opt;
    std::unordered_map<std::string, std::string> opts;
    for(int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if(arg[0] == '-') { // either just an opt or opt=val
            arg = arg.substr(1);

            if(expected_val) {
                LOGE() << "expected a value after opt " << last_opt;
                return std::nullopt;
            }

            std::size_t eqsgn_pos = arg.find('=');
            if(eqsgn_pos != std::string::npos) {
                auto opt_and_val = extract_opt_and_val(arg, eqsgn_pos);
                if(!opt_and_val) {
                    return std::nullopt;
                }
                opts[opt_and_val.value().first] = opt_and_val.value().second;
            }
            else { // just an opt
                auto opt = extract_opt(arg, eqsgn_pos);
                if(!opt) {
                    return std::nullopt;
                }
                expected_val = !expected_val;
                last_opt = opt.value();
            }
        }
        else { // just a val
            if(!expected_val) {
                LOGE() << "expected an option";
                return std::nullopt;
            }

            opts[last_opt] = arg;
            expected_val = !expected_val;
        }
    }

    const std::array<std::string, 2> nonopt_opts = {"w", "i"};
    for(auto opt: nonopt_opts) {
        if(!opts.count(opt)) {
            LOGE() << "missing option " << opt;
            return std::nullopt;
        }
    }

    return convert_opts(opts);
}

} // anonymous

int main(int argc, char** argv)
{
    uwmf::logger::filter() = uwmf::logger::log_level::VERBOSE;

    auto opts = parse_program_opts(argc, argv);
    if(!opts) {
        LOGE() << help();
        return -1;
    }

    LOGI() << "running UWMF with" << opts.value();

    uwmf::monochrome_image img(4, 4);
    uwmf::monochrome_image img2(4, 4);
    const uwmf::monochrome_image& img_ref = img;
    const uwmf::monochrome_image& img_ref2 = img2;

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            img(i,j) = 1;
            img2(i,j) = 2;
        }
    }

    for(auto [x, y, val]: img) {
        (void)x;
        (void)y;
        LOGI() << "px1 -> " << static_cast<int>(*val);
    }

    for(auto [x, y, val]: img) {
        (void)x;
        (void)y;
        *val = 11;
    }

    for(auto [x, y, val]: img_ref) {
        (void)x;
        (void)y;
        LOGI() << "px1 -> " << static_cast<int>(*val);
    }

    for(auto [px1, px2]: uwmf::make_image_zip(img, img2))
    {
        *(px1.value) = 55;
        *(px2.value) = 66;
    }

    for(auto [px1, px2]: uwmf::make_image_zip(img_ref, img_ref2))
    {
        LOGI() << "px1 -> " << static_cast<int>(*(px1.value));
        LOGI() << "px2 -> " << static_cast<int>(*(px2.value));
    }

    for(auto [px1, px2]: uwmf::make_image_zip(img, img2))
    {
        *(px1.value) = 11;
        *(px2.value) = 22;
    }

    const uwmf::monochrome_image const_img = img;
    const uwmf::monochrome_image const_img2 = img2;

    for(auto [px1, px2]: uwmf::make_image_zip(const_img, const_img2))
    {
        LOGI() << "px1 -> " << static_cast<int>(*(px1.value));
        LOGI() << "px2 -> " << static_cast<int>(*(px2.value));
    }

    uwmf::random real(0.2, 0.4);
    uwmf::random integral(0, 100);
    for(int i = 0; i < 100; i++) {
        LOGI() << "num -> " << real.generate();
        LOGI() << "num -> " << integral.generate();
    }

    auto png = *uwmf::read_png_image("../images/lena.png");
    uwmf::monochrome_image image(png.buffer, png.width, png.height);
    auto corrupt_image = fvin(image, 0.1);
    uwmf::write_png_image(corrupt_image.data(),
            corrupt_image.width(), corrupt_image.height(),
            "../images/lena_corrupted.png");

    return 0;
}
