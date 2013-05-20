cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts_net_udt)

set( DEPS ../dependencies)

add_definitions( -std=c++98 )

include_directories(
	${DEPS}/ # For UDT 
	../common-lib/include
)

add_definitions( -std=c++98 )

aux_source_directory( "impl/" lanarts_net_udt_src)

add_library( lanarts_net_udt
             STATIC
             ${lanarts_net_udt_src})


