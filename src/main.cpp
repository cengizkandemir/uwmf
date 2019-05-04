#include <cstddef>
#include <cstdlib>
#include <exception>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <unistd.h>

#include "image.h"
#include "logger.h"

namespace
{

constexpr int DEFAULT_WEIGHT_FALL_OFF = 4;
constexpr int DEFAULT_MINKOWSKI_EXPONENT = 1;

class program_options
{
        friend std::ostream& operator<<(std::ostream& out,
                const program_options& opts);
public:
    program_options()
        : k_(DEFAULT_WEIGHT_FALL_OFF, true)
        , p_(DEFAULT_MINKOWSKI_EXPONENT, true)
        , wsize_(-1, false)
        , input_("", false)
        , output_("")
    {
    }

    void k(int k)
    {
        k_.first = k;
        k_.second = true;
    }

    void p(int p)
    {
        p_.first = p;
        p_.second = true;
    }

    void wsize(int wsize)
    {
        wsize_.first = wsize;
        wsize_.second = true;
    }

    void input(std::string input)
    {
        input_.first = input;
        input_.second = true;
    }

    void output(std::string output)
    {
        output_ = output;
    }

    bool ok() const
    {
        return k_.second && p_.second && wsize_.second && input_.second;
    }

private:
    std::pair<int, bool> k_;
    std::pair<int, bool> p_;
    std::pair<int, bool> wsize_;
    std::pair<std::string, bool> input_;
    // optional
    std::string output_;
};

std::ostream& operator<<(std::ostream& out, const program_options& opts)
{
    out << "\n";
    out << "    k = " << opts.k_.first << "\n";
    out << "    p = " << opts.p_.first << "\n";
    out << "    w = " << opts.wsize_.first << "\n";
    out << "    i = " << opts.input_.first << "\n";
    out << "    o = " << opts.output_;

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

// TODO: consider better alternative to std::stoi
std::optional<program_options> parse_options(int argc, char** argv)
{
    LOGV() << "parse_options()";

    program_options opts;

    int curr_opt;
    while((curr_opt = getopt(argc, argv, ":k:p:w:i:o:h")) != -1) {
        switch(curr_opt) {
        case 'k':
            opts.k(std::stoi(optarg));
            break;
        case 'p':
            opts.p(std::stoi(optarg));
            break;
        case 'w':
            opts.wsize(std::stoi(optarg));
            break;
        case 'i':
            opts.input(std::string(optarg));
            break;
        case 'o':
            opts.output(std::string(optarg));
            break;
        case 'h':
            LOGI() << help();
            std::exit(0);
            break;
        case ':':
            LOGE() << "missing value for option -" << static_cast<char>(optopt);
            break;
        case '?':
            LOGE() << "invalid argument -" << optarg;
            break;
        default:
            ASSERT(false, "invalid getopt case");
            break;
        }
    }

    return opts.ok() ? std::optional(opts) : std::nullopt;
}

} // anonymous

int main(int argc, char** argv)
{
    logger::filter() = logger::log_level::VERBOSE;

    std::optional<program_options> opts;
    try {
        opts = parse_options(argc, argv);
    }
    catch(const std::exception& error) {
        LOGE() << error.what();
        LOGI() << help();
        return -1;
    }

    if(!opts) {
        LOGE() << help();
        return -1;
    }

    LOGD() << "parameters in use:";
    LOGD() << opts.value();

    std::size_t size_x = 16;
    std::size_t size_y = 19;
    monochrome_image img(size_x, size_y);
    monochrome_image img2(size_x, size_y);
    for(std::size_t y = 0; y < size_y; y++) {
        for(std::size_t x = 0; x < size_x; x++) {
            img(x, y) = 55;
            img2(x, y) = 66;
        }
    }

    LOGI() << img;

    for(const auto& [px1, px2]: image_pair_view(img, img2)) {
        LOGI() << "px1 -> " << static_cast<int>(px1.value);
        LOGI() << "px2 -> " << static_cast<int>(px2.value);
    }

    return 0;
}
