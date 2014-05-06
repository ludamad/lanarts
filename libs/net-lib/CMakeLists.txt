cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts_net)

set( DEPS ../../dependencies)

FIND_PACKAGE(SDL REQUIRED)

add_definitions( -std=c++98 )

include_directories(
	${SDL_INCLUDE_DIR} 
	${DEPS}/enet/include
	../common-lib/include
	../luawrap-lib/include
	${DEPS}/UnitTest++ 
)

add_definitions( -std=c++98 )

aux_source_directory( "impl/" lanarts_net_src)
aux_source_directory( "tests/" lanarts_net_tests_src)

add_library( lanarts_net
             STATIC
             ${lanarts_net_src})

	
add_executable( lanarts_net_tests
             ${lanarts_net_tests_src})

target_link_libraries(
	lanarts_net_tests
	lanarts_net
	pthread
	enet
    ${SDL_LIBRARY}
	lcommon
	UnitTest++
)
