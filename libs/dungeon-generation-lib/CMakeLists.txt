# Map generation primitives (areas, tunnels, BSP, polygon fills) and their Lua bindings
file(GLOB ldungeon_gen_sources CONFIGURE_DEPENDS src/*.cpp src/lua/*.cpp)
add_library(lanarts_dungeon_gen STATIC ${ldungeon_gen_sources})
target_include_directories(lanarts_dungeon_gen PUBLIC include PRIVATE include/ldungeon_gen)
target_link_libraries(lanarts_dungeon_gen PUBLIC lcommon libxmi PRIVATE lanarts::warnings)
