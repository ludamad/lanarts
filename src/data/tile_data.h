/*
 * tile_data.h
 *
 *  Created on: 2011-10-25
 *      Author: 100397561
 */

#ifndef TILE_DATA_H_
#define TILE_DATA_H_

#include "../display/GLImage.h"

enum {
	TILE_WALL = 0,
	TILE_STONE_WALL = 1,
	TILE_FLOOR = 2,
	TILE_SANDSTONE_0 = 3,
	TILE_SANDSTONE_1 = 4,
	TILE_SANDSTONE_2 = 5,
	TILE_SANDSTONE_3 = 6,
	TILE_SANDSTONE_4 = 7,
	TILE_MESH_0 = 8,
	TILE_MESH_1 = 9,
	TILE_MESH_2 = 10,
	TILE_MESH_3 = 11,
	TILE_MARBLE_1 = 12,
	TILE_MARBLE_2 = 13,
	TILE_MARBLE_3 = 14,
	TILE_MARBLE_4 = 15,
	TILE_MARBLE_5 = 16,
	TILE_MARBLE_6 = 17,
	TILE_CORRIDOR_FLOOR = 18,
	TILE_STAIR_UP = 19,
	TILE_STAIR_DOWN = 20
};

struct TileEntry {
	const char* name;
	GLImage img; 
	bool solid;
	TileEntry(const char* name, const char* fname, bool solid = false) 
		: name(name), img(fname), solid(solid){}

	void init(){
		printf("Loading tile '%s'\n", img.filename);
		init_GL_Image(&img, img.filename);
	}
};



extern TileEntry game_tile_data[];
extern size_t game_tile_n;

void init_tile_data();

#endif /* TILE_DATA_H_ */
