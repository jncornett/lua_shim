file ( GLOB SHIM_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" )

add_executable ( shim_tests ${SHIM_TEST_SOURCES} )
target_link_libraries ( shim_tests shim ${LUAJIT_LIBRARIES} )
set_property ( TARGET shim_tests PROPERTY CXX_STANDARD 11 )
