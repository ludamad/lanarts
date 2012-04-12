cmake_minimum_required( VERSION 2.8 )
PROJECT(lanartsTCP)

FIND_PACKAGE(BOOST REQUIRED)
FIND_PACKAGE(ASIO REQUIRED)

set( CMAKE_CXX_FLAGS_RELEASE "-O3" )
set( CMAKE_BUILD_TYPE Release)

include_directories(${BOOST_INCLUDE_DIR} ${ASIO_INCLUDE_DIR})

set( lanarts_tcp_src
	impl/connection.cpp
	impl/packet.cpp
	impl/ServerNetConnection.cpp
	impl/ClientNetConnection.cpp
	impl/SocketStream.cpp
	)

set( lanarts_tcp_include
	impl/ServerNetConnection.h
	impl/ClientNetConnection.h
	impl/SocketStream.h
	connection.h
	packet.h
)

add_library( lanarts_tcp
             ${lanarts_tcp_src}
             ${lanarts_tcp_include} )