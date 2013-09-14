/*
 * TileEntry.h:
 *  Represents a tile entry loaded from the yaml files.
 */

#include "TileEntry.h"

tile_id get_tile_by_name(const char* name);
TileEntry& get_tile_entry(tile_id id);

namespace res {
	TileEntry& tile(tile_id idx) {
		return get_tile_entry(idx);
	}

	TileEntry& tile(const char* name) {
		return tile(tileid(name));

	}
	TileEntry& tile(const std::string& name) {
		return tile(name.c_str());
	}

	tile_id tileid(const char* name) {
		return get_tile_by_name(name);
	}
	tile_id tileid(const std::string& name) {
		return tileid(name.c_str());
	}
}

