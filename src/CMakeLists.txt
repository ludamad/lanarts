cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)


include_directories(${SDL_INCLUDE_DIR} ${OpenGL_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ../src_yaml/yaml-cpp/include ../src_net/ ../src_rvo2/RVO2 ../src_lua)

set( lanarts_src

	collision_avoidance/CollisionAvoidance.cpp

	combat_logic/ai_logic.cpp
	combat_logic/attack_logic.cpp
	combat_logic/combat_logic.cpp
	combat_logic/movement_logic.cpp

	lua/lua_combatstats.cpp
	lua/lua_effectivestats.cpp

	stats/loaddata/load_armour_data.cpp
	stats/loaddata/load_spell_data.cpp
	display/loaddata/load_sprite_data.cpp
	levelgen/loaddata/load_dungeon_data.cpp
	stats/loaddata/load_effect_data.cpp
	objects/loaddata/load_enemy_data.cpp
	stats/loaddata/load_item_data.cpp
	stats/loaddata/load_itemgen_data.cpp
	gamestate/loaddata/load_tileset_data.cpp
	stats/loaddata/load_projectile_data.cpp
	gamestate/loaddata/load_settings_data.cpp
	stats/loaddata/load_weapon_data.cpp
	stats/loaddata/load_class_data.cpp
	display/loaddata/load_sprite_data.cpp
	data/yaml_util.cpp

	data/game_data.cpp
	data/loaddata/load_datafilenames.cpp

	display/display_shape.cpp
	display/display_text.cpp
	display/display_image.cpp
	display/font.cpp
	display/GLImage.cpp

	fov/fov.cpp
	fov/impl/permissive-fov.cc

	stats/combat_stats.cpp
	stats/effects.cpp
	stats/Equipment.cpp
	stats/Inventory.cpp
	stats/items.cpp
	stats/skills.cpp
	stats/stat_formulas.cpp
	stats/stats.cpp
	stats/SpellsKnown.cpp

	lua/lua_api.cpp
	lua/lua_collision_checks.cpp
	lua/lua_display_funcs.cpp
	lua/lua_object_funcs.cpp
	lua/lua_spelltarget_funcs.cpp
	lua/lua_gameinst.cpp
	lua/lua_gamestate.cpp

	pathfind/astar_pathfind.cpp
	pathfind/pathfind.cpp

	net/GameNetConnection.cpp
	main.cpp
	main_menu.cpp

	levelgen/areagen.h
	levelgen/enemygen.cpp
	levelgen/featuregen.cpp
	levelgen/GeneratedLevel.cpp
	levelgen/itemgen.cpp
	levelgen/lua_levelgen_funcs.cpp
	levelgen/levelgen.cpp
	levelgen/tunnelgen.cpp

	interface/console_description_draw.cpp
	lanarts_defines.cpp
	util/math_util.cpp
	util/game_replays.cpp
	util/mtwist.cpp
	lua/LuaValue.cpp
	objects/collision_filters.cpp

	gamestate/GameAction.cpp
	gamestate/GameInstSet.cpp
	gamestate/GameState.cpp
	gamestate/GameLevelState.cpp
	gamestate/GameTiles.cpp
	gamestate/GameView.cpp
	gamestate/GameWorld.cpp

	interface/GameChat.cpp
	interface/GameTextConsole.cpp
	interface/GameDialogs.cpp
	interface/GameHud.cpp
	interface/ActionBar.cpp
	interface/sidebar/Sidebar.cpp
	interface/sidebar/SidebarNavigator.cpp
	interface/sidebar/SpellsContent.cpp
	interface/sidebar/EnemiesSeenContent.cpp
	interface/sidebar/EquipmentContent.cpp
	interface/sidebar/InventoryContent.cpp
	interface/sidebar/StoreContent.cpp
	interface/sidebar/Minimap.cpp

	objects/CombatInstController.cpp
	gamestate/IOController.cpp
	objects/enemy/MonsterController.cpp
	objects/enemy/MonsterControllerActions.cpp
	objects/player/PlayerController.cpp
	
	objects/store/StoreInventory.cpp
	objects/store/StoreInst.cpp

	objects/AnimatedInst.cpp
	interface/ButtonInst.cpp

	objects/player/PlayerInst.cpp
	objects/player/PlayerInstActions.cpp
	objects/player/PlayerInstAttackActions.cpp
	objects/CombatGameInst.cpp
	objects/ScriptedInst.cpp
	objects/enemy/EnemyInst.cpp
	objects/FeatureInst.cpp
	objects/GameInst.cpp
	objects/GameInstRef.cpp
	objects/ItemInst.cpp
	objects/ProjectileInst.cpp
	)

set( lanarts_include

	collision_avoidance/CollisionAvoidance.h

	combat_logic/ai_logic.h
	combat_logic/attack_logic.h
	combat_logic/combat_logic.h
	combat_logic/movement_logic.h

	data/yaml_util.h

	stats/armour_data.h
	stats/class_data.h
	stats/effect_data.h
	objects/enemy/enemy_data.h
	stats/item_data.h
	data/lua_game_data.h
	data/game_data.h
	objects/scriptobject_data.h
	display/sprite_data.h
	levelgen/dungeon_data.h
	display/tile_data.h
	gamestate/tileset_data.h
	stats/weapon_data.h

	display/display.h
	display/GLImage.h

	fov/fov.h
	fov/impl/permissive-fov.h
	fov/impl/permissive-fov-cpp.h

	lua/lua_api.h
	lua/lua_impl_util.h


	stats/combat_stats.h
	stats/Inventory.h
	stats/items.h
	stats/itemgen_data.h
	stats/ItemsKnown.h
	stats/effects.h
	stats/Equipment.h
	stats/EnemiesSeen.h
	stats/skills.h
	stats/stat_formulas.h
	stats/stats.h
	stats/SpellsKnown.h

	pathfind/astar_pathfind.h
	pathfind/pathfind.h

	levelgen/areagen.h
	levelgen/enemygen.h
	levelgen/featuregen.h
	levelgen/GeneratedLevel.h
	levelgen/itemgen.h
	levelgen/levelgen.h
	levelgen/lua_levelgen_funcs.h
	levelgen/tunnelgen.h

	util/callback_util.h
	interface/console_description_draw.h
	lanarts_defines.h
	util/game_replays.h
	pathfind/gheap.h
	gamestate/hashset_util.h
	util/math_util.h
	util/mtwist.h
	objects/collision_filters.h

	net/GameNetConnection.h

	gamestate/GameAction.h
	gamestate/GameInstSet.h
	gamestate/GameState.h
	gamestate/GameLevelState.h
	gamestate/GameSettings.h
	gamestate/GameTiles.h
	gamestate/GameView.h
	gamestate/GameWorld.h

	gamestate/IOController.h
	objects/enemy/MonsterController.h
	objects/player/PlayerController.h
	objects/CombatInstController.h

	interface/GameChat.h
	interface/GameDialogs.h
	interface/GameHud.h
	interface/GameTextConsole.h
	interface/ActionBar.h
	interface/sidebar/EnemiesSeenContent.h
	interface/sidebar/EquipmentContent.h
	interface/sidebar/InventoryContent.h
	interface/sidebar/Sidebar.h
	interface/sidebar/SidebarContent.h
	interface/sidebar/SidebarNavigator.h
	interface/sidebar/StoreContent.h
	interface/sidebar/SpellsContent.h
	interface/sidebar/Minimap.h

	objects/AnimatedInst.h
	interface/ButtonInst.h

	objects/store/StoreInventory.h
	objects/store/StoreInst.h
	
	objects/CombatGameInst.h
	objects/GameInst.h
	objects/GameInstRef.h
	objects/player/PlayerInst.h
	objects/enemy/EnemyBehaviour.h
	objects/enemy/EnemyInst.h
	objects/FeatureInst.h
	objects/ScriptedInst.h
	objects/ProjectileInst.h
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
