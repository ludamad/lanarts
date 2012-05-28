/*
 * tile_data.h
 *
 *  Created on: 2011-10-25
 *      Author: 100397561
 */

#ifndef TILE_DATA_H_
#define TILE_DATA_H_

#include <vector>
#include "../display/GLImage.h"

struct TileEntry {
	const char* name;
	GLimage img; 
	TileEntry(const char* name, const char* fname)
		: name(name), img(fname){}

	void init(){
		init_GL_Image(&img, img.filename);
	}
};



extern std::vector<TileEntry> game_tile_data;

int get_tile_by_name(const char* name);

#endif /* TILE_DATA_H_ */
