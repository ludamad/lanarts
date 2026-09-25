# OpenGL drawing: images, fonts, animations, and the Lua bindings for them
file(GLOB ldraw_sources CONFIGURE_DEPENDS src/*.cpp src/lua/*.cpp src/opengl/*.cpp)
add_library(ldraw STATIC ${ldraw_sources})
target_include_directories(ldraw PUBLIC include PRIVATE include/ldraw)
target_link_libraries(ldraw
    PUBLIC lcommon lanarts::opengl imgui
    PRIVATE lanarts::freetype ${CMAKE_DL_LIBS} lanarts::warnings)
