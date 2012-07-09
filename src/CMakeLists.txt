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
	
	config/GameControlSettings.cpp

	data/init/load_armour_data.cpp
	data/init/load_spell_data.cpp
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
	display/display_text.cpp
	display/display.cpp
	display/font.cpp
	display/GLImage.cpp
	
	fov/fov.cpp
	fov/impl/permissive-fov.cc
	
	gamestats/Ability.cpp
	gamestats/combat_stats.cpp
	gamestats/effects.cpp
	gamestats/Equipment.cpp
	gamestats/Inventory.cpp
	gamestats/items.cpp
	gamestats/skills.cpp
	gamestats/stat_formulas.cpp
	gamestats/stats.cpp	
	gamestats/SpellsKnown.cpp	
			
	lua/lua_api.cpp
	lua/lua_collision_checks.cpp
	lua/lua_gameinst.cpp
	lua/lua_gamestate.cpp
	
	pathfind/astar_pathfind.cpp
	pathfind/pathfind.cpp
	
	world/net/GameNetConnection.cpp
	main.cpp
	main_menu.cpp
	
	procedural/enemygen.cpp
	procedural/featuregen.cpp
	procedural/GeneratedLevel.cpp
	procedural/itemgen.cpp
	procedural/levelgen.cpp
	procedural/tunnelgen.cpp
	
	util/content_draw_util.cpp
	util/math_util.cpp
	util/mtwist.cpp
	util/LuaValue.cpp
	util/world/collision_util.cpp

	world/GameAction.cpp
	world/GameInstSet.cpp
	world/GameState.cpp
	world/GameLevelState.cpp
	world/GameTiles.cpp
	world/GameView.cpp
	world/GameWorld.cpp
	
	world/ui/GameChat.cpp
	world/ui/GameTextConsole.cpp
	world/ui/GameDialogs.cpp
	world/ui/GameHud.cpp
	world/ui/detail/ActionBar.cpp
	world/ui/detail/Sidebar.cpp
	world/ui/detail/SidebarNavigator.cpp
	world/ui/detail/SpellsContent.cpp
	world/ui/detail/EnemiesSeenContent.cpp
	world/ui/detail/EquipmentContent.cpp
	world/ui/detail/InventoryContent.cpp
	world/ui/detail/Minimap.cpp
	
	world/controllers/CombatInstController.cpp
	world/controllers/MonsterController.cpp
	world/controllers/MonsterControllerActions.cpp
	world/controllers/PlayerController.cpp
	
	world/utility_objects/AnimatedInst.cpp
	world/utility_objects/ButtonInst.cpp
	world/utility_objects/TestInst.cpp
	
	world/objects/player_impl/PlayerInst.cpp
	world/objects/player_impl/PlayerInstActions.cpp
	world/objects/player_impl/PlayerInstAttackActions.cpp
	world/objects/CombatGameInst.cpp
	world/objects/ScriptedInst.cpp
	world/objects/EnemyInst.cpp
	world/objects/GameInst.cpp
	world/objects/GameInstRef.cpp
	world/objects/ItemInst.cpp
	world/objects/ProjectileInst.cpp
	)

set( lanarts_include

	collision_avoidance/CollisionAvoidance.h
	
	combat_logic/ai_logic.h
	combat_logic/attack_logic.h
	combat_logic/combat_logic.h
	combat_logic/movement_logic.h

	config/GameControlSettings.h
	
	data/init/yaml_util.h
	
	data/armour_data.h
	data/class_data.h
	data/effect_data.h
	data/enemy_data.h
	data/item_data.h
	data/lua_game_data.h
	data/game_data.h
	data/scriptobject_data.h
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
	lua/lua_impl_util.h
	
	gamestats/Ability.h	
	gamestats/combat_stats.h
	gamestats/Inventory.h
	gamestats/items.h
	gamestats/effects.h
	gamestats/Equipment.h
	gamestats/EnemiesSeen.h
	gamestats/skills.h
	gamestats/stat_formulas.h
	gamestats/stats.h
	gamestats/SpellsKnown.h
	
	pathfind/astar_pathfind.h
	pathfind/pathfind.h
	
	procedural/enemygen.h
	procedural/featuregen.h
	procedural/GeneratedLevel.h
	procedural/itemgen.h
	procedural/levelgen.h
	procedural/tunnelgen.h
	
	util/callback_util.h
	util/content_draw_util.h
	util/game_basic_structs.h
	util/gheap.h
	util/hashset_util.h
	util/math_util.h
	util/mtwist.h
	util/world/collision_util.h
	
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
	world/controllers/CombatInstController.h

	world/ui/GameChat.h
	world/ui/GameDialogs.h
	world/ui/GameHud.h
	world/ui/GameTextConsole.h
	world/ui/detail/ActionBar.h
	world/ui/detail/EnemiesSeenContent.h
	world/ui/detail/EquipmentContent.h
	world/ui/detail/InventoryContent.h
	world/ui/detail/Sidebar.h
	world/ui/detail/SidebarContent.h
	world/ui/detail/SidebarNavigator.h
	world/ui/detail/SpellsContent.h
	world/ui/detail/Minimap.h
	
	world/utility_objects/AnimatedInst.h
	world/utility_objects/ButtonInst.h
	world/utility_objects/TestInst.h
			
	world/objects/CombatGameInst.h	
	world/objects/GameInst.h
	world/objects/GameInstRef.h
	world/objects/PlayerInst.h
	world/objects/EnemyBehaviour.h
	world/objects/EnemyInst.h
	world/objects/ScriptedInst.h
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
