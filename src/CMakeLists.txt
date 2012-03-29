cmake_minimum_required( VERSION 2.8 )
PROJECT(base_LanARTS)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
include(FindFreetype)

set( CMAKE_CXX_FLAGS_RELEASE "-O3 -static-libgcc -static-libstdc++" )
set( CMAKE_BUILD_TYPE Release)

include_directories(${SDL_INCLUDE_DIR} ${OpenGL_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} yaml-cpp/include)

set( lanarts_src
	gamestats/Item.cpp
	
	data/init/load_sprite_data.cpp
	data/init/load_dungeon_data.cpp
	data/init/load_enemy_data.cpp
	data/init/load_item_data.cpp
	data/init/load_settings_data.cpp
	data/init/load_weapon_data.cpp
	data/init/load_class_data.cpp 
	data/init/load_sprite_data.cpp
	
	data/effect_data.cpp
	data/game_data.cpp
	data/item_data.cpp
	
	display/display.cpp
	display/GLImage.cpp
	
	fov/fov.cpp
	fov/impl/permissive-fov.cc
	
	gamestats/Ability.cpp
	gamestats/Stats.cpp
	
	pathfind/astar_pathfind.cpp
	pathfind/pathfind.cpp
	
	main.cpp
	
	procedural/enemygen.cpp
	procedural/featuregen.cpp
	procedural/GeneratedLevel.cpp
	procedural/itemgen.cpp
	procedural/levelgen.cpp
	procedural/mtwist.cpp
	procedural/tunnelgen.cpp
	
	util/draw_util.cpp
	util/font_util.cpp
	

	world/GameAction.cpp
	world/GameInstSet.cpp
	world/GameState.cpp
	world/GameLevelState.cpp
	world/GameTiles.cpp
	world/GameView.cpp
	world/GameHud.cpp
	
	world/controllers/MonsterController.cpp
	world/controllers/PlayerController.cpp
	
	world/objects/AnimatedInst.cpp
	world/objects/PlayerInst.cpp
	world/objects/PlayerInstActions.cpp
	world/objects/EnemyInst.cpp
	world/objects/ItemInst.cpp
	world/objects/BulletInst.cpp
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
	data/weapon_data.h
	
	display/display.h
	display/GLImage.h
	
	fov/fov.h
	fov/impl/permissive-fov.h
	fov/impl/permissive-fov-cpp.h
	
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
	procedural/pathfind.h
	procedural/tunnelgen.h
	
	util/draw_util.h
	util/font_util.h
	
	world/GameAction.h
	world/GameInstSet.h
	world/GameState.h
	world/GameLevelState.h
	world/GameSettings.h
	world/GameTiles.h
	world/GameView.h
	world/GameHud.h
	
	world/controllers/MonsterController.h
	world/controllers/PlayerController.h
	
	world/objects/GameInst.h
	
	world/objects/AnimatedInst.h
	world/objects/PlayerInst.h
	world/objects/EnemyInst.h
	world/objects/BulletInst.h
	world/objects/TestInst.h
)

add_subdirectory( yaml-cpp )

add_executable( lanarts
             ${lanarts_src}
             ${lanarts_include} )


target_link_libraries( lanarts
			yaml-cpp
			${OPENGL_gl_LIBRARY}
			${SDL_LIBRARY}
			${SDLIMAGE_LIBRARY}
			${SDLMAIN_LIBRARY}
			${FREETYPE_LIBRARIES}
			)