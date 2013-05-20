cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts_net_udt)

set( DEPS ../dependencies)

add_definitions( -std=c++98 -DLINUX  )

include_directories(
	${DEPS}/ # For UDT 
	../common-lib/include
	../luawrap-lib/include
	${DEPS}/UnitTest++ 
)

add_definitions( -std=c++98 )

aux_source_directory( "impl/" lanarts_net_udt_src)
aux_source_directory( "tests/" lanarts_net_udt_tests_src)

add_library( lanarts_net_udt
             STATIC
             ${lanarts_net_udt_src})

add_executable( lanarts_net_udt_tests
             ${lanarts_net_udt_tests_src})

target_link_libraries(
	lanarts_net_udt_tests
	lanarts_net_udt
	pthread
	udt
	lcommon
	UnitTest++
)
