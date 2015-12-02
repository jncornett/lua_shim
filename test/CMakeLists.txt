cmake_minimum_required(VERSION 2.8.12.1)
project (shim_tests CXX)

include ( FindLuaJIT.cmake )
include_directories ( .. )

file (
    GLOB TEST_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.cc"
)

add_executable ( tests ${TEST_FILES} )
