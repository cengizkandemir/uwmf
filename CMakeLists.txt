cmake_minimum_required(VERSION 3.0)

project(uwmf)

set(SOURCES
  src/utils.h
  src/utils.cpp
  src/logger.h
  src/logger.cpp
  src/image.h
  src/image.cpp
  src/main.cpp)

add_executable(uwmf ${SOURCES})

target_compile_options(uwmf PRIVATE
  -std=c++17
  -Wall
  -Wextra
  -Werror
  -Wshadow
  -Wpedantic
  -pedantic-errors)

install(TARGETS uwmf RUNTIME DESTINATION bin)
