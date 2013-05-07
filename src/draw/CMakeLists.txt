cmake_minimum_required( VERSION 2.6 )

PROJECT(project_ldraw)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ../)
set( LIBS ../../dependencies)

include_directories(
	include/ldraw/
	${SRC}/common/include
	${SRC}/common/include
	${SRC}/luawrap/include
	${LIBS}/UnitTest++
	${LIBS}/lua
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

set(LINKEDLIBS
	ldraw
	luawrap
	lcommon
	lua
	${OPENGL_gl_LIBRARY}
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${SDLMAIN_LIBRARY}
	${FREETYPE_LIBRARIES}
)

add_executable( ldraw_ex ${ldraw_examples} )
add_executable( ldraw_tests ${ldraw_tests} )

target_link_libraries( ldraw_ex ${LINKEDLIBS} )
target_link_libraries( ldraw_tests ${LINKEDLIBS} UnitTest++ )

enable_testing()
add_test(NAME ldraw_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/ldraw_tests )
