cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lua_enet)

set( lua_enet_src
    enet.c
)

include_directories("." "../enet/include" "../lua")

add_library( lua_enet STATIC
             ${lua_enet_src} )

target_link_libraries(lua_enet enet)
