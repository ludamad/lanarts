cmake_minimum_required( VERSION 2.6 )
PROJECT(project_udt)

set( udt_src
	api.cpp buffer.cpp cache.cpp ccc.cpp channel.cpp common.cpp core.cpp epoll.cpp list.cpp md5.cpp packet.cpp queue.cpp window.cpp
)

add_library( udt STATIC
             ${udt_src} )
