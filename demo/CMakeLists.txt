file ( GLOB DEMO_FILES ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" )

add_executable ( demo ${DEMO_FILES} )
target_link_libraries ( demo reg ${LUAJIT_LIBRARIES} )
