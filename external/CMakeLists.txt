if( NOT EXISTS "${CMAKE_SOURCE_DIR}/external/googletest/.git" )
  execute_process( COMMAND git submodule update --init --recursive)
endif()

add_subdirectory(googletest)
