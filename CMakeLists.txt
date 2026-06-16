cmake_minimum_required(VERSION 3.21)

project(marjoram)

add_library(marjoram INTERFACE)
target_include_directories(marjoram INTERFACE include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  set(CMAKE_BUILD_TYPE "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  find_package(Boost 1.58.0 REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS})

  # compiler flags
  add_compile_options(-fdiagnostics-color)
  add_compile_options(-ftemplate-backtrace-limit=0)
  add_compile_options(-pedantic)
  add_compile_options(-std=c++17)
  add_compile_options(-Wall)
  add_compile_options(-Wextra)
  add_compile_options(-Wpedantic)
  add_compile_options(-Wno-unused-local-typedefs)
  add_compile_options(-Wno-c++98-compat)
  add_compile_options(-Wno-padded)
  add_compile_options(-Wwrite-strings)

  include_directories("include")
  add_subdirectory(external)

  list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/external/sanitizers-cmake/cmake")

  # coveralls
  option(COVERALLS "Generate coveralls data" OFF)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/external/coveralls-cmake/cmake)
  if(COVERALLS)
    include(Coveralls)
    coveralls_turn_on_coverage()

    file(GLOB COVERAGE_SRCS "${PROJECT_SOURCE_DIR}/include/marjoram/*")
    coveralls_setup(
        "${COVERAGE_SRCS}" # The source files.
        ON                 # If we should upload.
    )
  endif()

  add_subdirectory(test)
  add_subdirectory(doxygen)
endif()
