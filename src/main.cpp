#include "logger.h"

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    logger::filter() = logger::log_level::DEBUG;

    LOGD() << "hello, world!";

    return 0;
}
