#include <algorithm>
#include <cctype>
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

enum class mode
{
    RESTORE,
    CORRUPT,
};

struct program_options
{
    mode m = mode::RESTORE;             // mode
    int k = DEFAULT_WEIGHT_FALL_OFF;    // weight fall-off
    int p = DEFAULT_MINKOWSKI_EXPONENT; // Minkowski exponent
    int w;                              // filtering window size
    double d;                           // corruption density
    std::string i;                      // input image
    std::string o;                      // output image
};

std::ostream& operator<<(std::ostream& out, const program_options& opts)
{
    out << "\n";

    switch(opts.m) {
    case mode::RESTORE:
        out << "    k = " << opts.k << "\n";
        out << "    p = " << opts.p << "\n";
        out << "    w = " << opts.w << "\n";
        break;
    case mode::CORRUPT:
        out << "    d = " << opts.d << "\n";
        break;
    default: ASSERT(false, "invalid operation"); break;
    }

    out << "    i = " << opts.i << "\n";
    out << "    o = " << opts.o;

    return out;
}

std::string help()
{
    std::stringstream ss;
    ss << "options:\n"
            << "    -m: mode, [c]orrupt/[r]estore (default: restore)\n"
            << "    -k: weight fall-off, restore mode only (default: "
            << DEFAULT_WEIGHT_FALL_OFF << ")\n"
            << "    -p: Minkowski exponent, restore mode only (default: "
            << DEFAULT_MINKOWSKI_EXPONENT << ")\n"
            << "    -w: filtering window size, restore mode only\n"
            << "    -d: corruption density, corrupt mode only\n"
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

template<typename T>
std::optional<T> to_numeric(const std::string& str)
{
    T result;
    auto [match, err] = std::from_chars(
            str.data(), str.data() + str.size(), result);

    if(err != std::errc()) {
        return std::nullopt;
    }

    return result;
}

template<>
std::optional<double> to_numeric<double>(const std::string& str)
{
    double value;
    try {
        value = std::stod(str);
    }
    catch(...) {
        return std::nullopt;
    }
    return value;
}

std::optional<mode> to_mode(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
            [] (unsigned char ch) { return std::tolower(ch); });

    if(str == "r" || str == "restore") {
        return mode::RESTORE;
    }
    else if(str == "c" || str == "corrupt") {
        return mode::CORRUPT;
    }

    return std::nullopt;
}

std::optional<program_options> convert_opts(
        const std::unordered_map<std::string, std::string>& opts_map)
{
    program_options opts;

    if(opts_map.count("m")) {
        auto m = to_mode(opts_map.at("m"));
        if(!m) {
            LOGE() << "failed to convert value for option m";
            return std::nullopt;
        }
        opts.m = m.value();
    }

    if(opts_map.count("k")) {
        auto k = to_numeric<int>(opts_map.at("k"));
        if(!k) {
            LOGE() << "failed to convert value for option k";
            return std::nullopt;
        }
        opts.k = k.value();
    }

    if(opts_map.count("p")) {
        auto p = to_numeric<int>(opts_map.at("p"));
        if(!p) {
            LOGE() << "failed to convert value for option p";
            return std::nullopt;
        }
        opts.p = p.value();
    }

    if(opts.m == mode::RESTORE) {
        ASSERT(opts_map.count("w"), "missing non optional option");
        auto w = to_numeric<int>(opts_map.at("w"));
        if(!w) {
            LOGE() << "failed to convert value for option w";
            return std::nullopt;
        }
        opts.w = w.value();
    }
    else if(opts.m == mode::CORRUPT) {
        ASSERT(opts_map.count("d"), "missing non optional option");
        auto d = to_numeric<double>(opts_map.at("d"));
        if(!d) {
            LOGE() << "failed to convert value for option d";
            return std::nullopt;
        }
        opts.d = d.value();
    }

    ASSERT(opts_map.count("i"), "missing non optional option");
    opts.i = opts_map.at("i");

    if(opts_map.count("o")) {
        opts.o = opts_map.at("o");
    }
    else {
        opts.o = opts.i;
        std::size_t dot_pos = opts.o.find_last_of('.');
        if(dot_pos == std::string::npos) {
            LOGE() << "missing file extension";
            return std::nullopt;
        }
        opts.o.insert(dot_pos,
                opts.m == mode::RESTORE ? "_restored" : "_corrupted");
    }

    return opts;
}

std::optional<std::string> find_missing_opts(
        const std::unordered_map<std::string, std::string>& curr_opts)
{
    const bool corrupt =
            curr_opts.count("m") && to_mode(curr_opts.at("m")) == mode::CORRUPT;
    const int opt_index = corrupt ? 1 : 0;
    const std::vector<std::vector<std::string>> nonopt_opts
            = {{"w", "i"}, {"d", "i"}};

    for(auto opt: nonopt_opts[opt_index]) {
        if(!curr_opts.count(opt)) {
            return opt;
        }
    }

    return std::nullopt;
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

    auto missing_opt = find_missing_opts(opts);
    if(missing_opt) {
        LOGE() << "missing option " << *missing_opt;
        return std::nullopt;
    }

    return convert_opts(opts);
}

} // anonymous

int main(int argc, char** argv)
{
    uwmf::logger::filter() = uwmf::logger::log_level::VERBOSE;

    auto result = parse_program_opts(argc, argv);
    if(!result) {
        LOGE() << help();
        return -1;
    }

    program_options opts = result.value();
    LOGI() << "running UWMF with" << opts;

    if(opts.m == mode::CORRUPT) {
        auto png = *uwmf::read_png_image(opts.i);
        uwmf::monochrome_image image(png.buffer, png.width, png.height);
        auto corrupt_image = fvin(image, opts.d);
        uwmf::write_png_image(corrupt_image.data(),
                corrupt_image.width(), corrupt_image.height(), opts.o);
    }

    return 0;
}
