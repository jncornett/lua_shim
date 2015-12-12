cmake_minimum_required ( VERSION 3.4 )
project ( Lua )

include ( cmake/FindLuaJIT.cmake )

option ( BUILD_UNIT_TESTS "build unit tests" ON )
option ( BUILD_DEMO "build demo" ON )
option ( ENABLE_COVERAGE "enable coverage" OFF )

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -ggdb" )
if ( ENABLE_COVERAGE )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --coverage" )
endif ( ENABLE_COVERAGE )

add_subdirectory ( shim )
add_subdirectory ( reg )

if ( BUILD_DEMO )
    add_subdirectory ( demo )
endif ( BUILD_DEMO )

