cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts_net)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)

include_directories(${SDL_INCLUDE_DIR} ${SDLNET_INCLUDE_DIR} )

add_definitions( -std=c++98 )

set( lanarts_net_tests
	tests/tests.h
	tests/tests.cpp
	tests/unit_tests.cpp
#	main.cpp
)

set( lanarts_net_src
	impl/ServerConnection.cpp
	impl/ClientConnection.cpp
	impl/lanarts_net.cpp
)

set( lanarts_net_include
	lanarts_net.h
	NetConnection.h
	impl/packet_util.h
	impl/ServerConnection.h
	impl/ClientConnection.h
)

add_library( lanarts_net
             STATIC
             ${lanarts_net_src})


