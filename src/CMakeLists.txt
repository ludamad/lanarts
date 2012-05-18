cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

include_directories(${SDL_INCLUDE_DIR} ${OpenGL_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ../src_yaml/yaml-cpp/include ../src_net/ ../src_rvo2/RVO2 ../src_lua)

set( lanarts_src
	gamestats/Item.cpp
	
	data/init/load_sprite_data.cpp
	data/init/load_dungeon_data.cpp
	data/init/load_enemy_data.cpp
	data/init/load_item_data.cpp
	data/init/load_tileset_data.cpp
	data/init/load_settings_data.cpp
	data/init/load_weapon_data.cpp
	data/init/load_class_data.cpp 
	data/init/load_sprite_data.cpp
	data/init/yaml_util.cpp
	
	data/effect_data.cpp
	data/game_data.cpp
	data/item_data.cpp
	data/lua_data.cpp
	
	display/display_util.cpp
	display/display.cpp
	display/font.cpp
	display/GLImage.cpp
	
	fov/fov.cpp
	fov/impl/permissive-fov.cc
	
	gamestats/Ability.cpp
	gamestats/Stats.cpp
	
	lua/LuaValue.cpp
	
	pathfind/astar_pathfind.cpp
	pathfind/pathfind.cpp
	
	world/net/GameNetConnection.cpp
	main.cpp
	
	procedural/enemygen.cpp
	procedural/featuregen.cpp
	procedural/GeneratedLevel.cpp
	procedural/itemgen.cpp
	procedural/levelgen.cpp
	procedural/mtwist.cpp
	procedural/tunnelgen.cpp
	
	util/collision_util.cpp
	util/math_util.cpp
	

	world/GameAction.cpp
	world/GameHud.cpp
	world/GameInstSet.cpp
	world/GameState.cpp
	world/GameLevelState.cpp
	world/GameTiles.cpp
	world/GameView.cpp
	world/GameWorld.cpp
	
	world/lua/lua_api.cpp
	world/lua/lua_gameinst.cpp
	world/lua/lua_gamestate.cpp
	world/lua/lua_item.cpp
	world/lua/lua_util.cpp
	
	world/controllers/MonsterController.cpp
	world/controllers/PlayerController.cpp
	
	world/objects/AnimatedInst.cpp
	world/objects/ButtonInst.cpp
	world/objects/PlayerInst.cpp
	world/objects/PlayerInstActions.cpp
	world/objects/EnemyInst.cpp
	world/objects/ItemInst.cpp
	world/objects/ProjectileInst.cpp
	world/objects/TestInst.cpp

	world/objects/GameInst.cpp
	)

set( lanarts_include
	
	data/init/yaml_util.h
	
	data/class_data.h
	data/effect_data.h
	data/enemy_data.h
	data/item_data.h
	data/game_data.h
	data/sprite_data.h
	data/dungeon_data.h
	data/tile_data.h
	data/tileset_data.h
	data/weapon_data.h
	data/lua_data.h
	
	display/display.h
	display/GLImage.h
	
	fov/fov.h
	fov/impl/permissive-fov.h
	fov/impl/permissive-fov-cpp.h
	
	lua/LuaValue.h
	
	gamestats/Ability.h
	gamestats/Effects.h
	gamestats/Inventory.h
	gamestats/Item.h
	gamestats/Stats.h
	
	pathfind/astar_pathfind.h
	pathfind/pathfind.h
	
	procedural/enemygen.h
	procedural/featuregen.h
	procedural/GeneratedLevel.h
	procedural/itemgen.h
	procedural/levelgen.h
	procedural/mtwist.h
	procedural/tunnelgen.h
	
	util/collision_util.h
	util/math_util.h
	util/game_basic_structs.h
	util/hashset_util.h
	
	world/net/GameNetConnection.h

	world/GameAction.h
	world/GameHud.h
	world/GameInstSet.h
	world/GameState.h
	world/GameLevelState.h
	world/GameSettings.h
	world/GameTiles.h
	world/GameView.h
	world/GameWorld.h
	
	world/controllers/MonsterController.h
	world/controllers/PlayerController.h
	
	world/objects/GameInst.h
	
	world/objects/AnimatedInst.h
	world/objects/ButtonInst.h
	world/objects/PlayerInst.h
	world/objects/EnemyInst.h
	world/objects/ProjectileInst.h
	world/objects/TestInst.h
	
	world/lua/lua_api.h
	world/lua/lua_util.h
)

add_executable( lanarts
             ${lanarts_src}
             ${lanarts_include} )


target_link_libraries( lanarts
			lanarts_tcp
			yaml-cpp
			RVO
			lua
			${OPENGL_gl_LIBRARY}
			${SDL_LIBRARY}
			${SDLIMAGE_LIBRARY}
			${SDLMAIN_LIBRARY}
			${FREETYPE_LIBRARIES}
			)
