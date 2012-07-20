/*
 * tile_data.h:
 *  Represents a tile entry loaded from the yaml files.
 */

#ifndef TILE_DATA_H_
#define TILE_DATA_H_

#include "GLImage.h"

#include "../lanarts_defines.h"
#include "../data/FilenameList.h"

struct TileEntry {
	std::string name;
	std::vector<GLimage> images;

	TileEntry(const std::string& name, const FilenameList& filenames) :
			name(name) {
		for (int i = 0; i < filenames.size(); i++) {
			images.push_back(GLimage(filenames[i]));
		}
	}
	void init() {
		for (int i = 0; i < images.size(); i++) {
			images[i].init();
		}
	}
	GLimage& img(int ind = 0) {
		return images.at(ind);
	}
};

extern std::vector<TileEntry> game_tile_data;

tile_id get_tile_by_name(const char* name);

#endif /* TILE_DATA_H_ */
