/*
 * Map.cpp:
 *  An abstract map format, where each square has a set of labels and a content number.
 *  16 bits are set out for labels. 8 are predefined, 8 are user defined.
 *  Provides efficient & flexible mechanisms for querying & operating on large areas.
 */

#include <lcommon/geometry.h>

#include <cstdio>

#include "Map.h"

namespace ldungeon_gen {

	Map::Map(const Size& size, const Square& fill_value) :
					Grid<Square>(size, fill_value) {
		/* Root parent group, allows group 0 to always be valid
		 * Things that apply to group 0 always apply to everything */
		make_group( BBox(Pos(0,0), size), -1);
	}

	Group& Map::make_group(const BBox& area, int parent_group) {
		int group_id = groups.size();
		groups.push_back(Group(group_id, parent_group, area));
		return groups.back();
	}
}
