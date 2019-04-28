#include <string>
#include <optional>
#include <utility>

#include <unistd.h>

#include "logger.h"


constexpr int DEFAULT_WEIGHT_FALL_OFF = 4;
constexpr int DEFAULT_MINKOWSKI_EXPONENT = 1;

struct program_options
{
    program_options()
        : k(DEFAULT_WEIGHT_FALL_OFF, true)
        , p(DEFAULT_MINKOWSKI_EXPONENT, true)
        , wsize(-1, false)
        , input("", false)
        , output("", false)
    {
    }

    bool ok() const
    {
        return k.second && p.second && wsize.second && input.second;
    }

    std::pair<int, bool> k;
    std::pair<int, bool> p;
    std::pair<int, bool> wsize;
    std::pair<std::string, bool> input;
    std::pair<std::string, bool> output;
};

namespace
{

// TODO: consider better alternative to std::stoi
std::optional<program_options> parse_options(int argc, char** argv)
{
    LOGV() << "parse_options()";

    program_options opts;

    int curr_opt;
    while((curr_opt = getopt(argc, argv, ":k:p:w:i:o:")) != -1) {
        switch(curr_opt) {
        case 'k':
            opts.k.first = std::stoi(optarg);
            opts.k.second = true;
            break;
        case 'p':
            opts.p.first = std::stoi(optarg);
            opts.p.second = true;
            break;
        case 'w':
            opts.wsize.first = std::stoi(optarg);
            opts.wsize.second = true;
            break;
        case 'i':
            opts.input.first = std::string(optarg);
            opts.input.second = true;
            break;
        case 'o':
            opts.output.first = std::string(optarg);
            opts.output.second = true;
            break;
        case ':':
            LOGE() << "missing value for opt -" << static_cast<char>(optopt);
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

}

int main(int argc, char** argv)
{
    logger::filter() = logger::log_level::VERBOSE;

    std::optional<program_options> opts =  parse_options(argc, argv);

    if(!opts) {
        LOGE() << "failed to parse command line arguments";
        return -1;
    }

    LOGD() << "parameters in use:";
    LOGD() << "k = " << opts.value().k.first;
    LOGD() << "p = " << opts.value().p.first;
    LOGD() << "w = " << opts.value().wsize.first;
    LOGD() << "i = " << opts.value().input.first;
    LOGD() << "o = " << opts.value().output.first;

    return 0;
}
