/*
 * game_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */

#include "game_data.h"

std::vector<TileEntry> game_tile_data;

std::vector<SpriteEntry> game_sprite_data;

std::vector<WeaponType> game_weapon_data;

void init_game_data(){
	//init_tile_data();
// 	init_sprite_data();
	load_tile_data("res/tiles.yaml");
	load_sprite_data("res/sprites.yaml");
	load_enemy_data("res/enemies.yaml");
	load_dungeon_data("res/levels.yaml");
}
