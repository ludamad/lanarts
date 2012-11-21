cmake_minimum_required( VERSION 2.6 )

set(CMAKE_BUILD_TYPE Debug)

PROJECT(project_ldraw)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ../)
set( LIBS ../../libs)

include_directories(
	${SDL_INCLUDE_DIR} 
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS} 
	${SRC} #/common 
	${LIBS} #/lua 
	${LIBS}/slb/include
)

set( ldraw_examples
  examples/examples.cpp
)

set( ldraw_src
    Animation.cpp
    Animation.h
    
    colour_constants.h
    
    Colour.cpp
    Colour.h
    
    DirectionalDrawable.cpp
    DirectionalDrawable.h
    
    display.cpp
    display.h
    
    DrawableBase.h
    Drawable.cpp
    Drawable.h
    
    draw.cpp
    draw.h
    
    DrawOptions.cpp
    DrawOptions.h
    
    Font.cpp
    Font.h
    
    Image.cpp
    Image.h
    
    ldraw_assert.h
    
    LuaDrawable.cpp
    LuaDrawable.h
    
    opengl/GLImage.h
    opengl/GLImage.cpp
  
    lua/lua_colour.h
    lua/lua_colour.cpp
    lua/lua_ldraw.h
    lua/lua_ldraw.cpp
    lua/lua_drawable.cpp
    lua/lua_font.h
    lua/lua_font.cpp
    lua/lua_drawoptions.h
    lua/lua_drawoptions.cpp
    lua/lua_image.h
    lua/lua_image.cpp
)

set( ldraw_tests
    tests/animation_tests.cpp
    tests/directionaldrawable_tests.cpp
    tests/DrawableMock.h
    tests/image_tests.cpp
    tests/ldraw_tests.cpp
    tests/ldraw_tests.h
    tests/lua_colour_tests.cpp
    tests/lua_drawable_tests.cpp
    tests/lua_drawoptions_tests.cpp
    tests/lua_font_tests.cpp
    tests/lua_image_tests.cpp
)

add_library( ldraw
             ${ldraw_src} )
add_executable( ldraw_ex ${ldraw_src} ${ldraw_examples} )


target_link_libraries( ldraw_ex
			"/home/adomurad/lanarts-git/lanarts_build/src/common/liblcommon.a"
			"/home/adomurad/lanarts-git/lanarts_build/libs/slb/libslb.a"
			"/home/adomurad/lanarts-git/lanarts_build/libs/lua/liblua.a"
#            lcommon
#            slb
#            lua
			${OPENGL_gl_LIBRARY}
			${SDL_LIBRARY}
			${SDLIMAGE_LIBRARY}
			${SDLMAIN_LIBRARY}
			${FREETYPE_LIBRARIES}
			)

add_executable( ldraw_tests ${ldraw_src} ${ldraw_tests} )
target_link_libraries( ldraw_tests
			"/home/adomurad/lanarts-git/lanarts_build/src/common/liblcommon.a"
			"/home/adomurad/lanarts-git/lanarts_build/libs/slb/libslb.a"
			"/home/adomurad/lanarts-git/lanarts_build/libs/lua/liblua.a"
#			lcommon
#            slb
#            lua
			${OPENGL_gl_LIBRARY}
			${SDL_LIBRARY}
			${SDLIMAGE_LIBRARY}
			${SDLMAIN_LIBRARY}
			${FREETYPE_LIBRARIES}
			)
