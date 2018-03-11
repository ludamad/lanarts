cmake_minimum_required( VERSION 2.6 )

PROJECT(project_ldraw)

FIND_LIBRARY(OpenGL REQUIRED)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  set(FREETYPE_INCLUDE_DIRS "/Users/adomurad/sources/freetype2/build/include/freetype" "/Users/adomurad/sources/freetype2/include/freetype" "/Users/adomurad/sources/freetype2/include"
      )

  set(FREETYPE_LIBRARIES "/Users/adomurad/sources/freetype2/build/libfreetype.a")

  set(SDL2_LIBRARIES "-s USE_SDL=2")
  set(SDL2_IMAGE_LIBRARIES "-s USE_SDL_IMAGE=2")
else ()
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(ARCH_LIBS 
        mingw32
		
	${CMAKE_SOURCE_DIR}/dependencies/binary/libSDL2.a
	${CMAKE_SOURCE_DIR}/dependencies/binary/libSDL2_image.a
	${CMAKE_SOURCE_DIR}/dependencies/binary/libSDL2main.a
	${CMAKE_SOURCE_DIR}/dependencies/binary/libSDL2_mixer.a
	mingw32  m dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid
        ws2_32 
    )
else()
    set(ARCH_LIBS 
	${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2IMAGE_LIBRARIES} 
    )
    INCLUDE(FindPkgConfig)
    include(FindFreetype)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
    PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
endif()

set( SRC ../)
set( DEPS ../../dependencies)


include_directories(
	include/ldraw/
	"${SRC}/common-lib/include"
	"${SRC}/luawrap-lib/include"
	${DEPS}/UnitTest++
	${DEPS}/lua
	${DEPS}/freetype/include
	"${DEPS}/sdl2/include"
	"${DEPS}/sdl2-image"
	${OpenGL_INCLUDE_DIRS} 		
	# ${FREETYPE_INCLUDE_DIRS} 
	freetype
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

         message(STATUS ${SDL2_LIBRARY})
set(LINKEDDEPS
	ldraw
	luawrap
	lcommon
	lua
		${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES}
	${OPENGL_gl_LIBRARY}
	# ${FREETYPE_LIBRARIES}
	freetype
)

add_executable( ldraw_ex ${ldraw_examples} )
#add_executable( ldraw_tests ${ldraw_tests} )
#
target_link_libraries( ldraw_ex ${LINKEDDEPS} )
#target_link_libraries( ldraw_tests ${LINKEDDEPS} UnitTest++ )
#
#enable_testing()
	#add_test(NAME ldraw_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/ldraw_tests )
