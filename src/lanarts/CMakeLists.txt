cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( SRC ../)
set( LIBS ../../libs)

set(CMAKE_BUILD_TYPE Debug)

#add_definitions(-DNDEBUG)

include_directories(
	${SDL_INCLUDE_DIR} 
	${OpenGL_INCLUDE_DIRS} 		
	${FREETYPE_INCLUDE_DIRS} 
	${SRC} #/common and /net
	${LIBS}/yaml-cpp/include 
	${LIBS} #/lua and /rvo2 
	${LIBS}/slb/include
	)

set( lanarts_src
	collision_avoidance/CollisionAvoidance.cpp
	
	data/game_data.cpp
	data/loaddata/load_datafilenames.cpp
	data/parse.cpp
	data/yaml_util.cpp
	data/FilenameList.cpp
	
	display/display_image.cpp
	display/GLImage.cpp
	display/loaddata/load_sprite_data.cpp
	display/loaddata/load_sprite_data.cpp
    
	draw/draw_statbar.cpp
	draw/parse_drawable.cpp
	
	fov/fov.cpp
	fov/impl/permissive-fov.cc
	
	gamestate/ActionQueue.cpp
	gamestate/GameAction.cpp
	gamestate/GameInstSet.cpp
	gamestate/GameLogger.cpp
	gamestate/GameLevelState.cpp
	gamestate/GameState.cpp
	gamestate/GameTiles.cpp
	gamestate/GameView.cpp
	gamestate/GameWorld.cpp
	gamestate/IOController.cpp
	gamestate/PlayerData.cpp
	
	gamestate/loaddata/load_settings_data.cpp
	gamestate/loaddata/load_tileset_data.cpp
	
	
	interface/ActionBar.cpp
	interface/ButtonInst.cpp
	interface/console_description_draw.cpp
	interface/components_draw.cpp
	interface/GameChat.cpp
	interface/GameChatCheatCodes.cpp
	interface/GameHud.cpp
	interface/GameTextConsole.cpp	
	interface/TextBoxInst.cpp
	interface/TextField.cpp
	
	interface/sidebar/ConfigContent.cpp
	interface/sidebar/EnemiesSeenContent.cpp
	interface/sidebar/EquipmentContent.cpp
	interface/sidebar/InventoryContent.cpp
	interface/sidebar/Minimap.cpp
	interface/sidebar/Sidebar.cpp
	interface/sidebar/SidebarNavigator.cpp
	interface/sidebar/SpellsContent.cpp
	interface/sidebar/StoreContent.cpp
	
	lanarts_defines.cpp
	
	levelgen/areagen.cpp
	levelgen/AreaTemplate.cpp
	levelgen/enemygen.cpp
	levelgen/featuregen.cpp
	levelgen/GeneratedLevel.cpp
	levelgen/itemgen.cpp
	levelgen/levelgen.cpp
	levelgen/loaddata/load_area_template.cpp
	levelgen/loaddata/load_dungeon_data.cpp
	levelgen/lua_levelgen_funcs.cpp
	levelgen/tunnelgen.cpp
	
	lua/lua_api.cpp
	lua/lua_collision_checks.cpp
	lua/lua_combatstats.cpp
	lua/lua_display_funcs.cpp
	lua/lua_effectivestats.cpp
	lua/lua_gameinst.cpp
	lua/lua_gamestate.cpp
	lua/lua_object_funcs.cpp
	lua/lua_spelltarget_funcs.cpp
	lua/lua_yaml.cpp
	lua/luaexpr.cpp
	
	menu/gameconfig_menu.cpp
	menu/lobby_menu.cpp
	menu/start_menu.cpp
	
	net/GameNetConnection.cpp
	
	objects/enemy/EnemyAIState.cpp
	objects/enemy/EnemyBehaviour.cpp
	objects/enemy/EnemyController.cpp
	objects/enemy/EnemyInst.cpp
	objects/enemy/MonsterControllerActions.cpp
	objects/enemy/MonsterController.cpp
	
	objects/AnimatedInst.cpp
	objects/collision_filters.cpp
	objects/CombatGameInst.cpp
	objects/CombatGameInstFunctions.cpp
	objects/FeatureInst.cpp
	objects/GameInst.cpp
	objects/GameInstRef.cpp
	objects/ItemInst.cpp
	objects/ProjectileInst.cpp
	objects/ScriptedInst.cpp
	objects/TeamRelations.cpp
	
	objects/loaddata/load_enemy_data.cpp
	
	objects/player/PlayerInstActions.cpp
	objects/player/PlayerInstAttackActions.cpp
	objects/player/PlayerInst.cpp
	
	objects/store/StoreInst.cpp
	objects/store/StoreInventory.cpp
	
	pathfind/astar_pathfind.cpp
	pathfind/pathfind.cpp
	
	stats/items/ItemEntry.cpp
	stats/items/items.cpp
	stats/items/EquipmentEntry.cpp
	stats/items/ProjectileEntry.cpp
	stats/items/WeaponEntry.cpp
	
	stats/loaddata/load_equipment_data.cpp
	stats/loaddata/load_class_data.cpp
	stats/loaddata/load_effect_data.cpp
	stats/loaddata/load_item_data.cpp
	stats/loaddata/load_itemgen_data.cpp
	stats/loaddata/load_projectile_data.cpp
	stats/loaddata/load_spell_data.cpp
	stats/loaddata/load_stats.cpp
	stats/loaddata/load_weapon_data.cpp
	
	stats/combat_stats.cpp
	stats/effects.cpp
	stats/EquipmentStats.cpp
	stats/Inventory.cpp
	stats/skills.cpp
	stats/SpellsKnown.cpp
	stats/stat_formulas.cpp
	stats/stats.cpp
	
	util/game_replays.cpp
	util/math_util.cpp
	
	main.cpp
)

set( lanarts_include
	lanarts_defines.h
	collision_avoidance/CollisionAvoidance.h

	data/BaseDataEntry.h
	data/game_data.h
	data/lua_game_data.h
	data/parse.h
	data/parse_with_defaults.h
	data/yaml_util.h

	display/display.h
	display/GLImage.h
	display/sprite_data.h
	display/tile_data.h

	draw/draw_statbar.h
	draw/colour_constants.h
	draw/parse_drawable.h

	fov/fov.h
	fov/impl/permissive-fov-cpp.h
	fov/impl/permissive-fov.h

	gamestate/ActionQueue.h
	gamestate/GameAction.h
	gamestate/GameInstSet.h
	gamestate/GameLevelState.h
	gamestate/GameLogger.h
	gamestate/GameSettings.h
	gamestate/GameState.h
	gamestate/GameTiles.h
	gamestate/GameView.h
	gamestate/GameWorld.h
	gamestate/hashset_util.h
	gamestate/IOController.h
	gamestate/PlayerData.h
	gamestate/tileset_data.h

	interface/ActionBar.h
	interface/ButtonInst.h
	interface/console_description_draw.h
	interface/components_draw.h
	interface/DrawCallbackInst.h
	interface/GameChat.h
	interface/GameHud.h
	interface/GameTextConsole.h
	interface/TextBoxInst.h
	interface/TextField.h

	interface/sidebar/ConfigContent.h
	interface/sidebar/EnemiesSeenContent.h
	interface/sidebar/EquipmentContent.h
	interface/sidebar/InventoryContent.h
	interface/sidebar/Minimap.h
	interface/sidebar/SidebarContent.h
	interface/sidebar/Sidebar.h
	interface/sidebar/SidebarNavigator.h
	interface/sidebar/SpellsContent.h
	interface/sidebar/StoreContent.h

	levelgen/areagen.h
	levelgen/AreaTemplate.h
	levelgen/dungeon_data.h
	levelgen/enemygen.h
	levelgen/featuregen.h
	levelgen/GeneratedLevel.h
	levelgen/itemgen.h
	levelgen/levelgen.h
	levelgen/lua_levelgen_funcs.h
	levelgen/tunnelgen.h

	lua/lua_api.h
	lua/lunar.h
	lua/lua_yaml.h
	lua/lua_gameinst.h
	lua/luaexpr.h

	menu/menus.h

	net/GameNetConnection.h

	objects/enemy/EnemyAIState.h
	objects/enemy/EnemyBehaviour.h
	objects/enemy/EnemyController.h
	objects/enemy/EnemyEntry.h
	objects/enemy/EnemyInst.h
	objects/enemy/MonsterController.h

	objects/player/PlayerInst.h

	objects/store/StoreInst.h
	objects/store/StoreInventory.h

	objects/AnimatedInst.h
	objects/collision_filters.h
	objects/CombatGameInstFunctions.h
	objects/CombatGameInst.h
	objects/FeatureInst.h
	objects/GameInst.h
	objects/GameInstRef.h
	objects/InstTypeEnum.h
	objects/ProjectileInst.h
	objects/ScriptedInst.h
	objects/scriptobject_data.h
	objects/TeamRelations.h

	pathfind/astar_pathfind.h
	pathfind/gheap.h
	pathfind/pathfind.h

	stats/loaddata/load_stats.h
	
	stats/items/EquipmentEntry.h
	stats/items/ItemEntry.h
	stats/items/ProjectileEntry.h
	stats/items/WeaponEntry.h
    
	stats/Attack.h
	stats/AllowedActions.h
	stats/class_data.h
	stats/combat_stats.h
	stats/effect_data.h
	stats/effects.h
	stats/EnemiesSeen.h
	stats/EquipmentStats.h
	stats/Inventory.h
	stats/itemgen_data.h
	stats/ItemsKnown.h
	stats/LuaAction.h
	stats/skills.h
	stats/SpellsKnown.h
	stats/stat_formulas.h
	stats/stats.h

	util/callback_util.h
	util/game_replays.h
	util/math_util.h
)

set( lanarts_tests
       tests/tests.h
       tests/tests.cpp
       tests/parse_tests.cpp

       tests/lua_tests.cpp

       tests/net/net_tests.cpp
       tests/net/adv_net_tests.cpp

       tests/stats/stat_serialize_tests.cpp
)


add_executable( lanarts
             ${lanarts_src}
             ${lanarts_include}
             ${lanarts_tests} )

set (libraries 
	lanarts_net
	yaml-cpp
	RVO
	ldraw
	lcommon
	slb
	lua
	${OPENGL_gl_LIBRARY}
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${SDLNET_LIBRARY}
	${SDLMAIN_LIBRARY}
	${FREETYPE_LIBRARIES}
)

target_link_libraries( lanarts 
    ${libraries}
)
