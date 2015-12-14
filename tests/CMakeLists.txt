file ( GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" )

add_executable ( tests ${TEST_SOURCES} )
target_link_libraries ( tests lua_shim ${LUAJIT_LIBRARIES} )
set_property ( TARGET tests PROPERTY CXX_STANDARD 11 )
