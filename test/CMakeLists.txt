cmake_minimum_required(VERSION 3.1)
project(marjoram)

file(GLOB test_SRC "*.cxx")
add_executable(marjoram_test ${test_SRC})
target_link_libraries(marjoram_test gtest_main)

add_custom_target(testrun
    COMMAND marjoram_test
    DEPENDS marjoram_test
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

# tests with sanitizers
find_package(Sanitizers)

add_executable(marjoram_test_san ${test_SRC})
target_link_libraries(marjoram_test_san gtest_main)
add_sanitizers(marjoram_test_san)
add_custom_target(testrun_san
    COMMAND marjoram_test_san
    DEPENDS marjoram_test_san
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
