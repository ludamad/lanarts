/*
 * levelgen.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "levelgen.h"
#include <cstdio>
#include "../world/GameState.h"
#include "../world/objects/PlayerInst.h"

const int TOO_MANY_ATTEMPTS = 1000;

//return false on failure
bool generate_room(const RoomGenSettings& rs, MTwist& mt, GeneratedLevel& level, const Region& r, int room_id) {
	if (!level.verify(r))
		return false;
	bool marked = (mt.rand(4) == 0);
	int mark = marked ? mt.rand(1,4) : 0;
	Sqr val = Sqr( true, false, false, (feature_t)mark, 0, room_id );
	level.set_region_with_perimeter(r, val, rs.room_padding);
	return true;
}

void generate_rooms(const RoomGenSettings& rs, MTwist& mt,
		GeneratedLevel& level) {
	std::vector<Room>& room_list = level.rooms();
	int w = level.width(), h = level.height();

	int pad = rs.room_padding;
	int minr = rs.min_size + pad * 2;
	int maxr = rs.max_size + pad * 2;

	for (int id = 1; id <= rs.amount_of_rooms; id++) {
		for (int attempts = 0;; attempts++) {

			int rw = mt.rand(minr, maxr+1);
			int rh = mt.rand(minr, maxr+1);

			int rx = mt.rand(w - 1 - rs.room_padding) | 1;
			int ry = mt.rand(h - 1 - rs.room_padding) | 1;

			Region r(rx, ry, rw, rh);
			Room room(
					Region(rx + pad, ry + pad, rw - pad * 2, rh - pad * 2), 0);

			if (generate_room(rs, mt, level, r, id)) {
				room_list.push_back(room);
				break;
			}
			if (attempts > TOO_MANY_ATTEMPTS)
				goto NoMoreRooms;// Goto below
		}
	}
	NoMoreRooms:				 // Come from above

	/*Room erosion/dilation*/
	int remove_corners_chance = 20;
	for (int i = 0; i < room_list.size(); i++) {
		Region r = room_list[i].room_region;
		bool remove_corners = mt.rand(100) <= remove_corners_chance;
		for (int y = r.y - 1; y < r.y + r.h + 1; y++)
			for (int x = r.x - 1; x < r.x + r.w + 1; x++) {
				Sqr& s = level.at(x,y);
				if (s.is_corner && remove_corners)
					s.passable = false;
			}
	}
}



void generate_level(const LevelGenSettings& ls, MTwist& mt,
		GeneratedLevel& level, GameState* gs) {
	level.initialize(ls.level_w, ls.level_h);
	generate_rooms(ls.rooms, mt, level);
	generate_tunnels(ls.tunnels, mt, level);
	generate_features(ls.features, mt, level, gs);
	generate_enemies(ls.enemies, mt, level, gs);
	generate_items(ls.items, mt, level, gs);
}
