cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_LIBRARY(OpenGL REQUIRED)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  # TODO(amitp): I don't know how to set this properly with add_compile_options
  set(SDL2_LIBRARIES "-s USE_SDL=2")
  set(SDL2_IMAGE_LIBRARIES "-s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=[\\\"png\\\"]")
  set(SDL2MIXER_LIBRARIES "-s USE_OGG=1 -s USE_VORBIS=1")
  set(FREETYPE_INCLUDE_DIRS "/Users/adomurad/sources/freetype2/build/include"  "/Users/adomurad/sources/freetype2/include/freetype")
  set(FREETYPE_LIBRARIES "/Users/adomurad/sources/freetype2/build/libfreetype.a")
else ()
    INCLUDE(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
    PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
    PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
    include(FindFreetype)
endif ()

set( LIBS ../libs)
set( DEPS ../dependencies)
set( BUILTDEPS ${CMAKE_BINARY_DIR}/dependencies )

add_definitions( -std=c++14 -Wno-deprecated-declarations -Wno-c++11-narrowing)

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
        oxygine-framework
        luayaml
	luawrap
	lanarts_net
        lpeg
        lfs
        enet
        lua_enet
        lua_box2d
#        ws2_32
#        mingw32
#        winmm
	RVO
        #physfs-static
        libxmi
        zlib
	${OPENGL_gl_LIBRARY}
	${FREETYPE_LIBRARIES}
        UnitTest++
        png
        backward_object
        ${BACKTRADE_DEP}
        ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2IMAGE_LIBRARIES} 
        # SDL2-mixer
)

target_link_libraries( lanarts 
    ${libraries}
)

enable_testing()
add_test(NAME lanarts_tests COMMAND 
    cd ${CMAKE_SOURCE_DIR} ;
    ${EXECUTABLE_OUTPUT_PATH}/lanarts --tests )
