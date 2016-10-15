# AD: Hacked for lanarts

# --------------------------------------------------------------
#  Box2D
# --------------------------------------------------------------
set(BOX2D_BUILD_STATIC ON)
set(BOX2D_BUILD_EXAMPLES OFF)
set(BOX2D_INSTALL OFF)

add_subdirectory(vendor/Box2D)

file(GLOB MODULE_SOURCES src/*.cpp src/bind/*.cpp src/bind/dub/*.cpp src)
include_directories(src/bind)
include_directories(include)
include_directories(vendor/Box2D)

add_library(lua_box2d STATIC ${MODULE_SOURCES})
target_link_libraries(lua_box2d lBox2D)

