#pragma once

#include <cstdio>
#include <sstream>

#include "utils.h"

#define LOG_IMPL(level)                         \
    if(level > logger::filter()) {              \
        ;                                       \
    }                                           \
    else                                        \
        logger(level).stream()

#define LOGD() LOG_IMPL(logger::log_level::DEBUG)
#define LOGI() LOG_IMPL(logger::log_level::INFO)
#define LOGW() LOG_IMPL(logger::log_level::WARNING)
#define LOGE() LOG_IMPL(logger::log_level::ERROR)

class logger
{
public:
    enum class log_level: int
    {
        DEBUG = 0,
        INFO,
        WARNING,
        ERROR
    };

    logger(log_level level)
        : level_(level)
    {
    }

    ~logger()
    {
        // we do not need flushing here, ostringstream is not tied to any device
        // and stderr is not buffered
        stream_ << "\n";
        std::fprintf(stderr, "%s", stream_.str().c_str());
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
};
