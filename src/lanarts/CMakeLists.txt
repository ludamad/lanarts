cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ..)
set( LIBS ../../libs)

include_directories(
	${SDL_INCLUDE_DIR} 
	${SDLIMAGE_INCLUDE_DIR}
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS}
	src/
	${SRC}/common/include
	${SRC}/draw/include
	${SRC}#/net
	${SRC}/luawrap/include
	${SRC}/sound/include
	${LIBS}/yaml-cpp/include 
	${LIBS}/lua 
	${LIBS}#/rvo2 
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
 
    src/levelgen/*
    src/levelgen/loaddata/*
 
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

       tests/net/*

       tests/stats/*
       tests/lua_api/*
)

file(GLOB lanarts_examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
       examples/*
)


add_library( lanartslib
             ${lanarts_src}
            )

add_executable( lanarts
	        src/main.cpp
	        )
	        
add_executable( lanarts_tests
             ${lanarts_tests} 
	        )

add_executable( lanarts_examples
             ${lanarts_examples} 
	        )


set (libraries 
    lanartslib
	ldraw
	lcommon
	lsound
	lua
	luawrap
	lanarts_net
	yaml-cpp
	RVO
	${OPENGL_gl_LIBRARY}
	${SDLMIXER_LIBRARY}
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
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
target_link_libraries( lanarts_examples
    ${libraries}
)
