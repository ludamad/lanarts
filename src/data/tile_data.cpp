/*
 * tile_data.cpp
 *
 *  Created on: 2011-10-26
 *      Author: 100397561
 */

#include "tile_data.h"

TileEntry game_tile_data[] = {
		TileEntry("floor","res/tiles/room_floor.bmp"),
		TileEntry("wall", "res/tiles/darkrock_wall.bmp"),
		TileEntry("black", "res/tiles/black.bmp"), };
size_t game_tile_n = sizeof(game_tile_data)/sizeof(TileEntry);

void init_tile_data(){
	for (int i = 0; i < game_tile_n; i++)
		game_tile_data[i].init();
}
