cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ..)
set( DEPS ../dependencies)
set( BUILTDEPS ${CMAKE_BINARY_DIR}/dependencies )

add_definitions( -std=c++98 )

if (${USE_LUAJIT})
    set( lua_includes ${DEPS}/luajit/src )
    set( lua_libraries luajit )
else()
    set( lua_includes ${DEPS}/lua  )
    set( lua_libraries lua )
endif()

include_directories(
	"."
	${SDL_INCLUDE_DIR} 
	${SDLIMAGE_INCLUDE_DIR}
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS}
	"${SRC}/common-lib/include"
	"${SRC}/draw-lib/include"
	"${SRC}/dungeon-generation-lib/include"
	${SRC} # for net
	"${SRC}/luawrap-lib/include"
	"${SRC}/sound-lib/include"
	${DEPS}/UnitTest++ 
	"${DEPS}/yaml-cpp/include"
	${lua_includes} 
	${DEPS} # for rvo2 & luasocket
	)

file(GLOB lanarts_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    include/* 
 
    collision_avoidance/*
 
    data/*
    data/loaddata/*
 
    display/*
    display/loaddata/*
 
    draw/*
    draw/loaddata/*
 
    fov/*
    fov/impl/*
 
    gamestate/*
    gamestate/loaddata/*
 
    interface/*
    interface/sidebar/*
 
    dungeon_generation/*
    dungeon_generation/loaddata/*
 
    lua/*
    lua_api/*
 
    menu/*
 
    net/*

    objects/*
    objects/enemy/*
    objects/loaddata/*
    objects/player/*
    objects/store/*

    pathfind/*

    stats/*
    stats/items/*
    stats/loaddata/*

    util/*

    lanarts_defines.cpp

	main.cpp
)

file(GLOB lanarts_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
       tests/*
)

add_executable( lanarts ${lanarts_src} ${lanarts_tests})

set (libraries 
	ldraw
	lcommon
	lanarts_dungeon_gen
	lsound
	${lua_libraries}
	luasocket
	luawrap
	lanarts_net
	yaml-cpp
	RVO
    libxmi
	${SDLIMAGE_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${SDLMIXER_LIBRARY}
    enet
	${SDL_LIBRARY}
	${SDLMAIN_LIBRARY}
	${FREETYPE_LIBRARIES}
        UnitTest++
)

target_link_libraries( lanarts 
    ${libraries}
)

enable_testing()
add_test(NAME lanarts_tests COMMAND 
    cd ${CMAKE_SOURCE_DIR} ;
    ${EXECUTABLE_OUTPUT_PATH}/lanarts --tests )
