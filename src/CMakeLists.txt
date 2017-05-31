cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_LIBRARY(OpenGL REQUIRED)
include(FindFreetype)

set( LIBS ../libs)
set( DEPS ../dependencies)
set( BUILTDEPS ${CMAKE_BINARY_DIR}/dependencies )

add_definitions( -std=c++11 -Wno-deprecated-declarations)

if (${USE_LUAJIT})
    set( lua_includes ${DEPS}/luajit/src )
else()
    set( lua_includes ${DEPS}/lua  )
endif()

set( lua_libraries lua )

include_directories(
    "."
    "${DEPS}/sdl2/include"
    "${DEPS}/sdl2-image"
    "${DEPS}/backward-cpp"
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
 
    display/*
 
    draw/*
 
    fov/*
    fov/impl/*
 
    gamestate/*
 
    interface/*
    interface/sidebar/*
 
    dungeon_generation/*
 
    lua/*
    lua_api/*
 
    menu/*
    net/*

    objects/*

    pathfind/*

    stats/*
    stats/items/*

    util/*

    lanarts_defines.cpp

    main.cpp
)

file(GLOB lanarts_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
       tests/*
)

add_executable( lanarts ${lanarts_src} ${lanarts_tests})
CHECK_INCLUDE_FILE( bfd.h HAS_BFD ) 
if ( HAS_BFD) 
    set( BACKTRADE_DEP bfd ) 
else()
    set( BACKTRADE_DEP "" ) 
endif()

set (libraries 
#        mingw32
	ldraw
	lcommon
	lanarts_dungeon_gen
	lsound
	${lua_libraries}
	luasocket
	yaml-cpp
        luayaml
	luawrap
	lanarts_net
        lpeg
        enet
        lua_enet
        lua_box2d
#        ws2_32
#        mingw32
#        winmm
	RVO
        #physfs-static
        libxmi
	${OPENGL_gl_LIBRARY}
	${FREETYPE_LIBRARIES}
        UnitTest++
        SDL2-image
        SDL2-mixer
        SDL2-static
#        /usr/i686-w64-mingw32/sys-root/mingw/lib/libSDL2_mixer.dll.a
#        /usr/i686-w64-mingw32/sys-root/mingw/lib/libSDL2_image.dll.a
#        /usr/i686-w64-mingw32/sys-root/mingw/lib/libSDL2.dll.a
        backward_object
        ${BACKTRADE_DEP}
)

target_link_libraries( lanarts 
    ${libraries}
)

enable_testing()
add_test(NAME lanarts_tests COMMAND 
    cd ${CMAKE_SOURCE_DIR} ;
    ${EXECUTABLE_OUTPUT_PATH}/lanarts --tests )
