find_package(Doxygen)
if(DOXYGEN_FOUND)
add_custom_target(doxygen
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Generating documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
