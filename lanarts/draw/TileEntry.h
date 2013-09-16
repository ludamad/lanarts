/*
 * TileEntry.h:
 *  Represents a tile entry loaded from the yaml files.
 */

#ifndef TILEENTRY_H_
#define TILEENTRY_H_

#include <ldraw/Image.h>

#include "data/FilenameList.h"

#include "lanarts_defines.h"

typedef std::vector<ldraw::Image> ImageList;

struct TileEntry {
	std::string name;
	ImageList images;

	const ldraw::Image& img(int ind = 0) const {
		return images.at(ind);
	}

	Size size() const {
		// Assert this for now so we can catch when its not true
		LANARTS_ASSERT(img().width() == TILE_SIZE && img().height() == TILE_SIZE);
		return Size(img().width(), img().height());
	}
};

extern std::vector<TileEntry> game_tile_data;

namespace res {
	TileEntry& tile(tile_id idx);

	TileEntry& tile(const char* name);
	TileEntry& tile(const std::string& name);

	tile_id tileid(const char* name);
	tile_id tileid(const std::string& name);
}

#endif /* TILEENTRY_H_ */
