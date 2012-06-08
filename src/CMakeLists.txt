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
	data/init/load_effect_data.cpp
	data/init/load_enemy_data.cpp
	data/init/load_item_data.cpp
	data/init/load_tileset_data.cpp
	data/init/load_settings_data.cpp
	data/init/load_weapon_data.cpp
	data/init/load_class_data.cpp 
	data/init/load_sprite_data.cpp
	data/init/yaml_util.cpp
	
	data/game_data.cpp	
	
	display/display_util.cpp
	display/display.cpp
	display/font.cpp
	display/GLImage.cpp
	
	fov/fov.cpp
	fov/impl/permissive-fov.cc
	
	gamestats/Ability.cpp
	gamestats/effects.cpp
	gamestats/Equipment.cpp
	gamestats/skills.h
	gamestats/stats.cpp
			
	lua/lua_api.cpp
	lua/lua_gameinst.cpp
	lua/lua_gamestate.cpp
	lua/lua_stats.cpp
	
	pathfind/astar_pathfind.cpp
	pathfind/pathfind.cpp
	
	world/net/GameNetConnection.cpp
	main.cpp
	
	procedural/enemygen.cpp
	procedural/featuregen.cpp
	procedural/GeneratedLevel.cpp
	procedural/itemgen.cpp
	procedural/levelgen.cpp
	procedural/tunnelgen.cpp
	
	util/collision_util.cpp
	util/math_util.cpp
	util/mtwist.cpp
	util/LuaValue.cpp

	world/GameAction.cpp
	world/GameInstSet.cpp
	world/GameState.cpp
	world/GameLevelState.cpp
	world/GameTiles.cpp
	world/GameView.cpp
	world/GameWorld.cpp
	
	world/ui/GameChat.cpp
	world/ui/GameDialogs.cpp
	world/ui/GameHud.cpp
	
	world/controllers/MonsterController.cpp
	world/controllers/PlayerController.cpp
	
	world/utility_objects/AnimatedInst.cpp
	world/utility_objects/ButtonInst.cpp
	world/utility_objects/TestInst.cpp
	
	world/objects/player_impl/PlayerInst.cpp
	world/objects/player_impl/PlayerInstActions.cpp
	world/objects/player_impl/PlayerInstAttackActions.cpp
	world/objects/EnemyInst.cpp
	world/objects/ItemInst.cpp
	world/objects/ProjectileInst.cpp

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
	
	display/display.h
	display/GLImage.h
	
	fov/fov.h
	fov/impl/permissive-fov.h
	fov/impl/permissive-fov-cpp.h
	
	lua/lua_api.h
	
	gamestats/Ability.h
	gamestats/Inventory.h
	gamestats/Item.h
	gamestats/effects.h
	gamestats/Equipment.h
	gamestats/skills.h
	gamestats/stats.h
	
	pathfind/astar_pathfind.h
	pathfind/pathfind.h
	
	procedural/enemygen.h
	procedural/featuregen.h
	procedural/GeneratedLevel.h
	procedural/itemgen.h
	procedural/levelgen.h
	procedural/tunnelgen.h
	
	util/collision_util.h
	util/callback_util.h
	util/game_basic_structs.h
	util/hashset_util.h
	util/math_util.h
	util/mtwist.h
	
	world/net/GameNetConnection.h

	world/GameAction.h
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

	world/ui/GameChat.h
	world/ui/GameDialogs.h
	world/ui/GameHud.h

	world/utility_objects/AnimatedInst.h
	world/utility_objects/ButtonInst.h
	world/utility_objects/TestInst.h
	
	world/objects/PlayerInst.h
	world/objects/EnemyBehaviour.h
	world/objects/EnemyInst.h
	world/objects/ProjectileInst.h
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
