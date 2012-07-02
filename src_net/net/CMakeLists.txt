cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts_tcp)

FIND_PACKAGE(Boost REQUIRED)

add_definitions(-DBOOST_ALL_NO_LIB=1)

set( CMAKE_CXX_FLAGS_RELEASE "-O3" )
set( CMAKE_BUILD_TYPE Release)

include_directories(../../src_asio/include ${Boost_INCLUDE_DIR} )

set( lanarts_tcp_src
	impl/connection.cpp
	impl/packet.cpp
	impl/ServerNetConnection.cpp
	impl/ClientNetConnection.cpp
	impl/SocketStream.cpp
	impl/timing.cpp
	)

set( lanarts_tcp_include
	impl/ServerNetConnection.h
	impl/ClientNetConnection.h
	impl/SocketStream.h
	connection.h
	packet.h
	timing.h
)

add_library( lanarts_tcp
             ${lanarts_tcp_src}
             ${lanarts_tcp_include} 
)
