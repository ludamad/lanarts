cmake_minimum_required( VERSION 2.6 )
PROJECT(project_udt)

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( udt_src
	api.cpp buffer.cpp cache.cpp ccc.cpp channel.cpp common.cpp core.cpp epoll.cpp list.cpp md5.cpp packet.cpp queue.cpp window.cpp
)

set( udt_src_test 
    tests/test.cpp
)

add_library( udt STATIC
             ${udt_src} )

add_executable(udt_test ${udt_src_test})

target_link_libraries( 
        udt_test
        udt
        pthread
)

# Enable tests
enable_testing()


