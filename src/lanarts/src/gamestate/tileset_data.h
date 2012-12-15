/*
 * tileset_data.h:
 *	Information for the various tile-sets of the game
 */

#ifndef TILESET_DATA_H_
#define TILESET_DATA_H_

#include <vector>
#include <string>
#include "display/GLImage.h"
#include "lanarts_defines.h"

struct TilesetEntry {
	std::string name;
	Range floor, wall, corridor, altfloor, altwall, altcorridor;

	TilesetEntry(
			const std::string& name,
			int minfloor, int maxfloor,
			int minwall, int maxwall,
			int mincorridor, int maxcorridor,
			int minaltfloor, int maxaltfloor,
			int minaltwall, int maxaltwall,
			int minaltcorridor, int maxaltcorridor) :
			name(name),
			floor(minfloor,maxfloor),
			wall(minwall,maxwall),
			corridor(mincorridor, maxcorridor),
			altfloor(minaltfloor,maxaltfloor),
			altwall(minaltwall,maxaltwall),
			altcorridor(minaltcorridor,maxaltcorridor){

	}
};

extern std::vector<TilesetEntry> game_tileset_data;

tileset_id get_tileset_by_name(const char* name);

#endif /* TILESET_DATA_H_ */
