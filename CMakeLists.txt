cmake_minimum_required(VERSION 3.0)

project(uwmf)

# TODO: use only release libraries for zlib and libpng
set(LIBPGN_LIBRARY_NAME "libpng16")
if(MSVC)
  set(ZLIB_LIBRARY_NAME "zlibstaticd.lib")
  set(LIBPGN_LIBRARY_NAME "${LIBPGN_LIBRARY_NAME}_staticd.lib")
else()
  set(ZLIB_LIBRARY_NAME "libz.a")
  set(LIBPGN_LIBRARY_NAME "${LIBPGN_LIBRARY_NAME}.a")
endif()

set(INSTALL_PREFIX ${CMAKE_BINARY_DIR}/external)
set(DEP_INTERM_INCLUDE_DIR ${INSTALL_PREFIX}/include)

include(ExternalProject)

set(ZLIB zlib-1.2.11)
set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/external/${ZLIB})
set(ZLIB_CMAKE_ARGS
  -D SKIP_INSTALL_ALL=OFF
  -D CMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
)
ExternalProject_Add(${ZLIB}
  PREFIX ${ZLIB}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${ZLIB_DIR}
  CMAKE_ARGS ${ZLIB_CMAKE_ARGS}
)


set(LIBPNG libpng-1.6.37)
set(LIBPNG_DIR ${CMAKE_SOURCE_DIR}/external/${LIBPNG})
set(LIBPNG_CMAKE_ARGS
  -D PNG_SHARED=OFF
  -D PNG_BUILD_ZLIB=ON
  -D ZLIB_INCLUDE_DIR=${ZLIB_DIR}
  -D ZLIB_CONF_DIR=${DEP_INTERM_INCLUDE_DIR}
  -D CMAKE_PROJECT_libpng_INCLUDE=${CMAKE_SOURCE_DIR}/libpng.cmake
  -D CMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
  -D CMAKE_INSTALL_LIBDIR=${INSTALL_PREFIX}/lib
)
ExternalProject_Add(${LIBPNG}
  PREFIX ${LIBPNG}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${LIBPNG_DIR}
  CMAKE_ARGS ${LIBPNG_CMAKE_ARGS}
)

# not necessarily needed for static linkage
ExternalProject_Add_StepDependencies(${LIBPNG} build ${ZLIB})

set(ZLIB_LIBRARY ${INSTALL_PREFIX}/lib/${ZLIB_LIBRARY_NAME})
set(LIBPNG_LIBRARY ${INSTALL_PREFIX}/lib/${LIBPGN_LIBRARY_NAME})

set(SOURCES
  src/utils.h
  src/utils.cpp
  src/logger.h
  src/logger.cpp
  src/image.h
  src/image.cpp
  src/png_image.h
  src/png_image.cpp
  src/math_utils.h
  src/math_utils.cpp
  src/metrics.h
  src/metrics.cpp
  src/main.cpp
)

add_executable(uwmf ${SOURCES})

add_dependencies(uwmf ${ZLIB} ${LIBPNG})

target_include_directories(uwmf PRIVATE ${DEP_INTERM_INCLUDE_DIR})

target_compile_features(uwmf PRIVATE cxx_std_17)

set(LIBRARIES
  ${LIBPNG_LIBRARY}
  ${ZLIB_LIBRARY}
)

if(MSVC)
  set(COMPILER_OPTIONS
    /Zi
    /permissive-
    /WX
    /W4
  )
else()
  set(COMPILER_OPTIONS
    -Wall
    -Wextra
    -Werror
    -Wshadow
    -Wpedantic
    -pedantic-errors
    -ggdb
    -g3
    -fsanitize=address
  )
  set(LIBRARIES
    ${LIBRARIES}
    asan
  )
endif()

target_compile_options(uwmf PRIVATE ${COMPILER_OPTIONS})

target_link_libraries(uwmf ${LIBRARIES})

install(TARGETS uwmf RUNTIME DESTINATION bin)
