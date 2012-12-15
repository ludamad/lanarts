/*
 * TileEntry.h:
 *  Represents a tile entry loaded from the yaml files.
 */

#ifndef TILEENTRY_H_
#define TILEENTRY_H_

#include <ldraw/Image.h>

#include "data/FilenameList.h"

#include "lanarts_defines.h"
#include "GLImage.h"

struct TileEntry {
	std::string name;
	std::vector<ldraw::Image> images;

	const ldraw::Image& img(int ind = 0) const {
		return images.at(ind);
	}

	Dim size() const {
		// Assert this for now so we can catch when its not true
		LANARTS_ASSERT(img().width() == TILE_SIZE && img().height() == TILE_SIZE);
		return Dim(img().width(), img().height());
	}
};

extern std::vector<TileEntry> game_tile_data;

tile_id get_tile_by_name(const char* name);
TileEntry& get_tile_entry(tile_id id);

#endif /* TILEENTRY_H_ */
