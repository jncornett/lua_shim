file ( GLOB SHIM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} "*.hpp" )

add_library ( reg INTERFACE )
set_property ( TARGET reg PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries ( reg INTERFACE shim )

if ( BUILD_UNIT_TESTS )
    add_subdirectory ( tests )
endif ( BUILD_UNIT_TESTS )
