# Box2D 2.3.0 with dub-generated Lua bindings (lua-box2d), adapted for lanarts
file(GLOB_RECURSE box2d_sources CONFIGURE_DEPENDS vendor/Box2D/Box2D/*.cpp)
add_library(box2d STATIC ${box2d_sources})
target_include_directories(box2d PUBLIC vendor/Box2D)

file(GLOB lua_box2d_sources CONFIGURE_DEPENDS src/*.cpp src/bind/*.cpp src/bind/dub/*.cpp)
add_library(lua_box2d STATIC ${lua_box2d_sources})
target_include_directories(lua_box2d PRIVATE src/bind PUBLIC include)
target_compile_options(lua_box2d PRIVATE -Wno-deprecated-declarations)
target_link_libraries(lua_box2d PUBLIC box2d lanarts::opengl)
