# Drawing API with no-op implementations, for headless builds
file(GLOB ldraw_sources CONFIGURE_DEPENDS src/*.cpp src/lua/*.cpp src/opengl/*.cpp)
add_library(ldraw STATIC ${ldraw_sources})
target_include_directories(ldraw PUBLIC include PRIVATE include/ldraw)
target_link_libraries(ldraw PUBLIC lcommon PRIVATE lanarts::warnings)
