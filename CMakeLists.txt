cmake_minimum_required ( VERSION 3.4 )
project ( Lua )

include ( cmake/FindLuaJIT.cmake )

option ( ENABLE_COVERAGE "" ON )

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -ggdb" )
if ( ENABLE_COVERAGE )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage" )
endif ( ENABLE_COVERAGE )

add_subdirectory ( lua )
add_subdirectory ( tests )
