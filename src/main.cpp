#include <array>
#include <charconv>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "image.h"
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
    ASSERT(!str.empty(), "trying to convert an empty string");

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
        opts.o.insert(opts.o.find('.'), "_restored");
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

    return -1;

    std::size_t size_x = 16;
    std::size_t size_y = 19;
    uwmf::monochrome_image img(size_x, size_y);
    uwmf::monochrome_image img2(size_x, size_y);
    uwmf::monochrome_image img3(size_x, size_y);
    for(std::size_t y = 0; y < size_y; y++) {
        for(std::size_t x = 0; x < size_x; x++) {
            img(x, y) = 55;
            img2(x, y) = 66;
            img3(x, y) = 77;
        }
    }

    LOGI() << img;

    for(const auto [x, y, val]: img) {
        LOGI() << "px1 -> " << static_cast<int>(val);
    }

    const auto& img_ref = img;
    const auto& img_ref2 = img;

    for(const auto [x, y, val]: img_ref) {
        LOGI() << "px1 -> " << static_cast<int>(val);
    }

    for(const auto [first, second]: uwmf::make_image_zip(img_ref, img_ref2)) {
        LOGI() << "px1 -> " << static_cast<int>(first.value);
    }

    for(const auto& [px1, px2, px3]:
                uwmf::make_image_zip(img, img2, img3)) {
        LOGI() << "px1 -> " << static_cast<int>(px1.value);
        LOGI() << "px2 -> " << static_cast<int>(px2.value);
        LOGI() << "px3 -> " << static_cast<int>(px3.value);
    }

    for(auto [px1, px2, px3]:
                uwmf::make_image_zip(img, img2, img3)) {
        LOGI() << "px1 -> " << static_cast<int>(px1.value);
        LOGI() << "px2 -> " << static_cast<int>(px2.value);
        LOGI() << "px3 -> " << static_cast<int>(px3.value);
    }

    LOGI() << "mse: " << uwmf::mse(img, img2);
    LOGI() << "psnr: " << uwmf::psnr(img, img2);

    uwmf::monochrome_png_image png_img("../images/lena.png");

    auto first = png_img.raw_data();
    LOGD() << "data: " << png_img.raw_data().size() << '\n';
    png_img.write("../images/lena_2.png");

    uwmf::monochrome_png_image png_img2("../images/lena_2.png");
    auto second = png_img2.raw_data();

    LOGD() << "equal? " << (first == second);

    uwmf::monochrome_image img4(png_img.raw_data(), png_img.width(),
            png_img.height());

    LOGD() << "width: " << img4.width() << ", height: " << img4.height();
    return 0;
}
