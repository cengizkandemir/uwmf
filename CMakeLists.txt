cmake_minimum_required(VERSION 3.0)

project(uwmf)

set(SOURCES
    src/utils.h
    src/logger.h
    src/main.cpp)

add_executable(uwmf ${SOURCES})

install(TARGETS uwmf RUNTIME DESTINATION bin)
