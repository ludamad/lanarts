cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_LIBRARY(SDL2 REQUIRED)
FIND_LIBRARY(SDL2_image REQUIRED)
FIND_LIBRARY(SDL2_mixer REQUIRED)
FIND_LIBRARY(OpenGL REQUIRED)
include(FindFreetype)

set( LIBS ../libs)
set( DEPS ../dependencies)
set( BUILTDEPS ${CMAKE_BINARY_DIR}/dependencies )

add_definitions( -std=c++11)

if (${USE_LUAJIT})
    set( lua_includes ${DEPS}/luajit/src )
else()
    set( lua_includes ${DEPS}/lua  )
endif()

set( lua_libraries lua )

include_directories(
    "."
    ${SDL_INCLUDE_DIR} 
    ${SDLIMAGE_INCLUDE_DIR}
    ${OpenGL_INCLUDE_DIRS} 		
    ${FREETYPE_INCLUDE_DIRS}
    "${LIBS}/common-lib/include"
    "${LIBS}/draw-lib/include"
    "${LIBS}/dungeon-generation-lib/include"
    ${LIBS} # for net
    "${LIBS}/luawrap-lib/include"
    "${LIBS}/sound-lib/include"
    ${DEPS}/UnitTest++ 
    "${DEPS}/yaml-cpp/include"
    "${DEPS}/physfs/src"
    "${DEPS}/physfs/extras"
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
        luayaml
	luawrap
	lanarts_net
	yaml-cpp
	RVO
        #physfs-static
        libxmi
	${OPENGL_gl_LIBRARY}
	${SDLMIXER_LIBRARY}
        enet
        #	${SDLMAIN_LIBRARY}
	${FREETYPE_LIBRARIES}
        UnitTest++
        SDL2-image
        SDL2-mixer
        SDL2-static
)

target_link_libraries( lanarts 
    ${libraries}
)

enable_testing()
add_test(NAME lanarts_tests COMMAND 
    cd ${CMAKE_SOURCE_DIR} ;
    ${EXECUTABLE_OUTPUT_PATH}/lanarts --tests )
