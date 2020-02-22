cmake_minimum_required(VERSION 3.0)

project(uwmf)

set(ZLIB zlib-1.2.11)
set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/external/${ZLIB})
include(ExternalProject)
ExternalProject_Add(${ZLIB}
  PREFIX ${ZLIB}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${ZLIB_DIR}
  CONFIGURE_COMMAND ${ZLIB_DIR}/./configure --static
  BUILD_COMMAND make
  INSTALL_COMMAND "")

ExternalProject_Get_property(${ZLIB} BINARY_DIR)
set(ZLIB_LIBRARY ${BINARY_DIR}/libz.a)
set(LIBPNG libpng-1.6.37)
set(LIBPNG_DIR ${CMAKE_SOURCE_DIR}/external/${LIBPNG})
set(LIBPNG_CMAKE_ARGS
  -D PNG_SHARED=OFF
  -D PNG_BUILD_ZLIB=ON
  -D ZLIB_INCLUDE_DIR=${ZLIB_DIR})
ExternalProject_Add(${LIBPNG}
  PREFIX ${LIBPNG}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${LIBPNG_DIR}
  CMAKE_ARGS ${LIBPNG_CMAKE_ARGS}
  INSTALL_COMMAND "")

# not necessarily needed for static linkage
ExternalProject_Add_StepDependencies(${LIBPNG} build ${ZLIB})

ExternalProject_Get_property(${LIBPNG} BINARY_DIR)
set(LIBPNG_LIBRARY ${BINARY_DIR}/libpng16.a)

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

target_include_directories(uwmf PRIVATE ${LIBPNG_DIR})

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

target_link_libraries(uwmf asan ${ZLIB_LIBRARY} ${LIBPNG_LIBRARY})

install(TARGETS uwmf RUNTIME DESTINATION bin)
