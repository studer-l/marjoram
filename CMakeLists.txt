cmake_minimum_required(VERSION 3.1)

project(marjoram)

enable_testing()
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.58.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(marjoram INTERFACE)

target_include_directories(marjoram INTERFACE include)

# compiler flags
add_compile_options(-fdiagnostics-color)
add_compile_options(-ftemplate-backtrace-limit=0)
add_compile_options(-pedantic)
add_compile_options(-std=c++14)
add_compile_options(-W)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-local-typedefs)
add_compile_options(-Wno-c++98-compat)
add_compile_options(-Wno-padded)
add_compile_options(-Wwrite-strings)

include_directories("include")
add_subdirectory(external)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/sanitizers-cmake/cmake"
  ${CMAKE_MODULE_PATH})

# coveralls
option(COVERALLS "Generate coveralls data" OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/external/coveralls-cmake/cmake)
if (COVERALLS)
    include(Coveralls)
    coveralls_turn_on_coverage()

    file(GLOB COVERAGE_SRCS "${PROJECT_SOURCE_DIR}/include/marjoram/*")
    # Create the coveralls target.
    coveralls_setup(
        "${COVERAGE_SRCS}" # The source files.
        ON                 # If we should upload.
    )
endif(COVERALLS)

add_subdirectory(test)
add_subdirectory(doxygen)
