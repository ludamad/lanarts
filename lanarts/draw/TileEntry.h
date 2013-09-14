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

struct TileLayout {
	Range image_range;
	/* Does this tile explicitly state if it is solid ? */
	bool use_tile_solidity, solidity;
	TileLayout() {
		use_tile_solidity = false;
		solidity = false;
	}
};

struct TileLayoutRules {
	// 9 orientations depending on surrounding tiles
	// Empty if a simple tile (only 'rest' is used)
	std::vector<Range> orientations;
	// Used when simple tile, or when there are no surrounding tiles
	Range rest;
};

struct TileEntry {
	std::string name;
	ImageList images;
	TileLayoutRules layout_rules;

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
