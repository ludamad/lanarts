cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts_net_enet)

set( DEPS ../dependencies)

FIND_PACKAGE(SDL REQUIRED)

add_definitions( -std=c++98 )

include_directories(
	${DEPS}/enet/include
	../common-lib/include
	../luawrap-lib/include
	${DEPS}/UnitTest++ 
)


add_definitions( -std=c++98 )

aux_source_directory( "impl/" lanarts_net_enet_src)
aux_source_directory( "tests/" lanarts_net_enet_tests_src)

add_library( lanarts_net_enet
             STATIC
             ${lanarts_net_enet_src})

	
add_executable( lanarts_net_enet_tests
             ${lanarts_net_enet_tests_src})

target_link_libraries(
	lanarts_net_enet_tests
	lanarts_net_enet
	pthread
	enet
    ${SDL_LIBRARY}
	lcommon
	UnitTest++
)
