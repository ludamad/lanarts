/*
 * tile_data.h
 *
 *  Created on: 2011-10-25
 *      Author: 100397561
 */

#ifndef TILE_DATA_H_
#define TILE_DATA_H_

#include "../display/GLImage.h"

struct TileEntry {
	const char* name;
	GLImage img;
	TileEntry(const char* name, const char* fname) : name(name), img(fname){}
	void init(){
		printf("Loading image '%s'\n", img.filename);
		init_GL_Image(&img, img.filename);
	}
};



extern TileEntry game_tile_data[];
extern size_t game_tile_n;

void init_tile_data();

#endif /* TILE_DATA_H_ */
