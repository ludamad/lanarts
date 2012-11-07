cmake_minimum_required( VERSION 2.6 )

set(CMAKE_BUILD_TYPE Debug)

PROJECT(project_ldraw)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

include_directories(
	${SDL_INCLUDE_DIR} 
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS} 
	../ #/common 
)

set( ldraw_examples
  examples/examples.cpp
)

set( ldraw_src
  font_data.h
  font_data.cpp

  DrawOptions.h
  DrawOptions.cpp

  opengl/GLImage.h
  opengl/GLImage.cpp

  colour_constants.h

  display.h
  display.cpp
  
  draw.h
  draw.cpp

  Colour.h
  Colour.cpp

  Image.h
  Image.cpp
)

#add_library( ldraw
#             ${ldraw_src} )
add_executable( ldraw_ex ${ldraw_src} ${ldraw_examples} )

target_link_libraries( ldraw_ex
			/home/adomurad/eclipsecommon/liblcommon.a
			${OPENGL_gl_LIBRARY}
			${SDL_LIBRARY}
			${SDLIMAGE_LIBRARY}
			${SDLMAIN_LIBRARY}
			${FREETYPE_LIBRARIES}
			)
