file ( GLOB SHIM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} "*.hpp" )

add_library ( shim INTERFACE )
set_property ( TARGET shim PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} )

if ( BUILD_UNIT_TESTS )
    add_subdirectory ( tests )
endif ( BUILD_UNIT_TESTS )
