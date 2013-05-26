cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ..)
set( DEPS ../dependencies)

add_definitions( -std=c++98 )

include_directories(
	${SDL_INCLUDE_DIR} 
	${SDLIMAGE_INCLUDE_DIR}
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS}
	src/
	"${SRC}/common-lib/include"
	"${SRC}/draw-lib/include"
	${SRC} # for net
	"${SRC}/luawrap-lib/include"
	"${SRC}/sound-lib/include"
	${DEPS}/UnitTest++ 
	"${DEPS}/yaml-cpp/include"
	${DEPS}/lua 
	${DEPS} # for rvo2 & luasocket
	)

file(GLOB lanarts_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    src/include/* 
 
    src/collision_avoidance/*
 
    src/data/*
    src/data/loaddata/*
 
    src/display/*
    src/display/loaddata/*
 
    src/draw/*
    src/draw/loaddata/*
 
    src/fov/*
    src/fov/impl/*
 
    src/gamestate/*
    src/gamestate/loaddata/*
 
    src/interface/*
    src/interface/sidebar/*
 
    src/dungeon_generation/*
    src/dungeon_generation/loaddata/*
 
    src/lua/*
    src/lua_api/*
 
    src/menu/*
 
    src/net/*

    src/objects/*
    src/objects/enemy/*
    src/objects/loaddata/*
    src/objects/player/*
    src/objects/store/*

    src/pathfind/*

    src/stats/*
    src/stats/items/*
    src/stats/loaddata/*

    src/util/*

    src/lanarts_defines.cpp
)

file(GLOB lanarts_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
       tests/*
)

file(GLOB lanarts_runner RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
       standalone/*
)


add_library( lanartslib STATIC
             ${lanarts_src}
            )

add_executable( lanarts
	        src/main.cpp
	        )
	        
add_executable( lanarts_tests
             ${lanarts_tests} 
	        )

add_executable( lanarts_runner
             ${lanarts_runner} 
	        )


set (libraries 
	lanartslib
	ldraw
	lcommon
	lsound
	lua
	luasocket
	luawrap
	lanarts_net_enet
	yaml-cpp
	RVO
	${OPENGL_gl_LIBRARY}
	${SDLMIXER_LIBRARY}
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
        enet
	${SDLNET_LIBRARY}
	${SDLMAIN_LIBRARY}
	${FREETYPE_LIBRARIES}
)

target_link_libraries( lanarts 
    ${libraries}
)

target_link_libraries( lanarts_tests
    ${libraries}
	UnitTest++
)
target_link_libraries( lanarts_runner
    ${libraries}
)

enable_testing()
add_test(NAME lanarts_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/lanarts_tests )
