/*
 * tile_data.cpp
 *
 *  Created on: 2011-10-26
 *      Author: 100397561
 */

#include "tile_data.h"

TileEntry game_tile_data[] = {
		TileEntry("wall", "res/tiles/darkrock_wall.bmp"),
		TileEntry("stone_wall", "res/tiles/stone_wall.bmp"),
		TileEntry("floor","res/tiles/room_floor.bmp"),
		TileEntry("sandfloor0","res/tiles/sandstone/sandstone_floor0.png"),
		TileEntry("sandfloor1","res/tiles/sandstone/sandstone_floor1.png"),
		TileEntry("sandfloor2","res/tiles/sandstone/sandstone_floor2.png"),
		TileEntry("sandfloor3","res/tiles/sandstone/sandstone_floor3.png"),
		TileEntry("sandfloor4","res/tiles/sandstone/sandstone_floor4.png"),
		TileEntry("mesh0","res/tiles/mesh/mesh0.png"),
		TileEntry("mesh1","res/tiles/mesh/mesh1.png"),
		TileEntry("mesh2","res/tiles/mesh/mesh2.png"),
		TileEntry("mesh3","res/tiles/mesh/mesh3.png"),
		TileEntry("marb1","res/tiles/marble/marble_floor1.png"),
		TileEntry("marb2","res/tiles/marble/marble_floor2.png"),
		TileEntry("marb3","res/tiles/marble/marble_floor3.png"),
		TileEntry("marb4","res/tiles/marble/marble_floor4.png"),
		TileEntry("marb5","res/tiles/marble/marble_floor5.png"),
		TileEntry("marb6","res/tiles/marble/marble_floor6.png"),
		TileEntry("corridor_floor","res/tiles/corridor_floor.bmp"),
};
size_t game_tile_n = sizeof(game_tile_data)/sizeof(TileEntry);

void init_tile_data(){
	for (int i = 0; i < game_tile_n; i++)
		game_tile_data[i].init();
}
