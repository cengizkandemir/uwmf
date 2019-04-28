#pragma once

#include <cstdio>
#include <sstream>

#include "utils.h"

#define LOG_IMPL(level)                         \
    if(level < logger::filter()) {              \
        ;                                       \
    }                                           \
    else                                        \
        logger(level).stream()

#define LOGV() LOG_IMPL(logger::log_level::VERBOSE)
#define LOGD() LOG_IMPL(logger::log_level::DEBUG)
#define LOGI() LOG_IMPL(logger::log_level::INFO)
#define LOGW() LOG_IMPL(logger::log_level::WARNING)
#define LOGE() LOG_IMPL(logger::log_level::ERROR)

class logger
{
public:
    enum class log_level: int
    {
        VERBOSE = 0,
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    logger(log_level level)
        : level_(level)
    {
        stream_ << "[" << log_level_as_str() << "] ";
    }

    ~logger()
    {
        stream_ << std::endl;
        std::fprintf(stderr, "%s", stream_.str().c_str());
        std::fflush(stderr);
    }

    DELETE_COPY_AND_ASSIGN(logger);

    std::ostringstream& stream()
    {
        return stream_;
    }

    static log_level& filter()
    {
        static log_level fltr = logger::log_level::INFO;
        return fltr;
    }
    
private:
    log_level level_;
    std::ostringstream stream_;


    std::string log_level_as_str() const
    {
        switch(level_) {
        case log_level::VERBOSE:
            return "VERBOSE";
        case log_level::DEBUG:
            return "DEBUG";
        case log_level::INFO:
            return "INFO";
        case log_level::WARNING:
            return "WARNING";
        case log_level::ERROR:
            return "ERROR";
        default:
            ASSERT(false, "invalid log_level_as_str case");
            break;
        }

        return "";
    }
};
