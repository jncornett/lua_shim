cmake_minimum_required(VERSION 2.8.12.1)
project (demo CXX)

include ( ../test/FindLuaJIT.cmake )
include_directories ( .. )

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb -O0" )

file (
    GLOB DEMO_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.cc"
)

add_executable ( demo ${DEMO_FILES} )
