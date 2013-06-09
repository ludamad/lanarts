cmake_minimum_required( VERSION 2.6 )

PROJECT(project_ldraw)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ../)
set( DEPS ../dependencies)

add_definitions( -std=c++98 )

include_directories(
	include/ldraw/
	"${SRC}/common-lib/include"
	"${SRC}/luawrap-lib/include"
	${DEPS}/UnitTest++
	${DEPS}/lua
	${SDL_INCLUDE_DIR} 
	${SDLIMAGE_INCLUDE_DIR} 
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS} 
)

file(GLOB ldraw_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    src/* 
    src/lua/* 
    src/opengl/* 
    include/draw/* 
)

set( ldraw_examples
    examples/examples.cpp
)
file(GLOB ldraw_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    tests/*
)

add_library( ldraw
		STATIC
             ${ldraw_src} )

set(LINKEDDEPS
	ldraw
	luawrap
	lcommon
	lua
	${OPENGL_gl_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${SDL_LIBRARY}
	${SDLMAIN_LIBRARY}
	${FREETYPE_LIBRARIES}
)

add_executable( ldraw_ex ${ldraw_examples} )
add_executable( ldraw_tests ${ldraw_tests} )

target_link_libraries( ldraw_ex ${LINKEDDEPS} )
target_link_libraries( ldraw_tests ${LINKEDDEPS} UnitTest++ )

enable_testing()
add_test(NAME ldraw_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/ldraw_tests )
