cmake_minimum_required(VERSION 3.1)

project(marjoram)

# Settings
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.58.0)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message(FATAL_ERROR)
endif()


# Sanitizer settings for testrun_san target
#set(SANITIZE_UNDEFINED ON)
#set(SANITIZE_ADDRESS ON)
#set(SANITIZE_MEMORY ON)

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

add_subdirectory(test)
add_subdirectory(doxygen)
