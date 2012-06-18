/*
 * levelgen.cpp:
 *  Contains procedures related to generating a full dungeon level
 */

#include <cstdio>

#include "../data/dungeon_data.h"

#include "../world/objects/PlayerInst.h"

#include "../world/GameState.h"

#include "GeneratedLevel.h"
#include "shapefill.h"
#include "levelgen.h"

const int TOO_MANY_ATTEMPTS = 1000;

//return false on failure
bool generate_room(const RoomGenSettings& rs, MTwist& mt, GeneratedLevel& level,
		const Region& r, int room_id) {
	if (!level.verify(r))
		return false;
	bool marked = (mt.rand(4) == 0);
	int mark = marked ? mt.rand(1, 4) : 0;
	Sqr val = Sqr(true, false, false, (feature_t)mark, 0, room_id);
	level.set_region_with_perimeter(r, val, rs.room_padding);
//	Region reg(r.x,r.y, 10, 10);
//	level.set_circle_with_perimeter(reg, val, rs.room_padding);
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

			int rw = mt.rand(minr, maxr + 1);
			int rh = mt.rand(minr, maxr + 1);

			int rx = mt.rand(w - 1 - rs.room_padding) | 1;
			int ry = mt.rand(h - 1 - rs.room_padding) | 1;

			Region r(rx, ry, rw, rh);
			Room room(Region(rx + pad, ry + pad, rw - pad * 2, rh - pad * 2),
					0);

			if (generate_room(rs, mt, level, r, id)) {
				room_list.push_back(room);
				break;
			}
			if (attempts > TOO_MANY_ATTEMPTS)
				goto NoMoreRooms;
			// Goto below
		}
	}
	NoMoreRooms: // Come from above

	/*Room erosion/dilation*/
	int remove_corners_chance = 20;
	for (int i = 0; i < room_list.size(); i++) {
		Region r = room_list[i].room_region;
		bool remove_corners = mt.rand(100) <= remove_corners_chance;
		for (int y = r.y - 1; y < r.y + r.h + 1; y++)
			for (int x = r.x - 1; x < r.x + r.w + 1; x++) {
				Sqr& s = level.at(x, y);
				if (s.is_corner && remove_corners)
					s.passable = false;
			}
	}
}

GameLevelState* generate_level(int roomid, MTwist& mt, GeneratedLevel& level,
		GameState* gs) {
	DungeonBranch& branch = game_dungeon_data[DNGN_MAIN_BRANCH];
	const LevelGenSettings& ls = branch.level_data[roomid];
	const RoomGenSettings& rs = ls.rooms;

	level.initialize(ls.level_w, ls.level_h, rs.solid_fill);

	int LEVEL_BORDER_PAD = 10;
	int lw = std::min(128, ls.level_w + LEVEL_BORDER_PAD), lh = std::min(128,
			ls.level_h + LEVEL_BORDER_PAD);
//	lw = 128, lh = 128;
	GameLevelState* newlvl = new GameLevelState(roomid, DNGN_MAIN_BRANCH,

			roomid, lw * TILE_SIZE, lh * TILE_SIZE, ls.wander);

	GameLevelState* prevlvl = gs->get_level(); //Save level context
	gs->set_level(newlvl); //Set level context to new level

	printf("level.init RNG state at %d numbers\n", mt.amount_of_randoms);
	generate_rooms(rs, mt, level);
	printf("ROOMS state at %d numbers\n", mt.amount_of_randoms);
	generate_tunnels(ls.tunnels, mt, level);
	printf("TUNNELS state at %d numbers\n", mt.amount_of_randoms);
	generate_features(ls.features, mt, level, gs);
	printf("FEATURES state at %d numbers\n", mt.amount_of_randoms);
	generate_enemies(ls.enemies, mt, level, gs);
	printf("ENEMIES state at %d numbers\n", mt.amount_of_randoms);
	generate_items(ls.items, mt, level, gs);
	printf("ITEMS state at %d numbers\n", mt.amount_of_randoms);
	newlvl->rooms = level.rooms();

	gs->set_level(prevlvl); //Restore level context

	return newlvl;
}
