cmake_minimum_required(VERSION 3.0)

project(uwmf)

set(SOURCES
  src/utils.h
  src/utils.cpp
  src/logger.h
  src/logger.cpp
  src/image.h
  src/image.cpp
  src/math_utils.h
  src/math_utils.cpp
  src/main.cpp)

add_executable(uwmf ${SOURCES})

target_compile_options(uwmf PRIVATE
  -std=c++17
  -Wall
  -Wextra
  -Werror
  -Wshadow
  -Wpedantic
  -pedantic-errors
  -ggdb
  -g3
  -fsanitize=address)

target_link_libraries(uwmf asan)

install(TARGETS uwmf RUNTIME DESTINATION bin)
