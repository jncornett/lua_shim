file ( GLOB REG_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" )

add_executable ( reg_tests ${REG_TEST_SOURCES} )
target_link_libraries ( reg_tests reg ${LUAJIT_LIBRARIES} )
set_property ( TARGET reg_tests PROPERTY CXX_STANDARD 11 )
