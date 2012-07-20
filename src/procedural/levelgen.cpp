/*
 * levelgen.cpp:
 *  Contains procedures related to generating a full dungeon level
 */

#include <cstdio>

#include "../data/dungeon_data.h"

#include "../objects/player/PlayerInst.h"

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
	int nrooms = mt.rand(rs.amount_of_rooms);
	Range sizerange(rs.size.min + pad * 2, rs.size.max + pad * 2);

	for (int id = 1; id <= nrooms; id++) {
		for (int attempts = 0;; attempts++) {

			int rw = mt.rand(sizerange);
			int rh = mt.rand(sizerange);

			int rx = mt.rand(1, w - rw - rs.room_padding) | 1;
			int ry = mt.rand(1, h - rh - rs.room_padding) | 1;

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

static const LayoutGenSettings& choose_random_layout(MTwist& mt,
		const LevelGenSettings& ls) {
	int layout_index = mt.rand(ls.layouts.size());
	return ls.layouts[layout_index];
}

GameLevelState* generate_level(int roomid, MTwist& mt, GeneratedLevel& level,
		GameState* gs) {
	DungeonBranch& branch = game_dungeon_data[DNGN_MAIN_BRANCH];

	const LevelGenSettings& ls = branch.level_data[roomid];
	const ContentGenSettings& content = ls.content;
	const LayoutGenSettings& layout = choose_random_layout(mt, ls);

	const std::vector<RoomGenSettings>& roomsettings = layout.rooms;

	int w = mt.rand(layout.width), h = mt.rand(layout.height);
	bool wandering = content.enemies.wandering;

	level.initialize(w, h, layout.solid_fill);

	int LEVEL_BORDER_PAD = 10;
	int lw = std::min(128, w + LEVEL_BORDER_PAD), lh = std::min(128,
			h + LEVEL_BORDER_PAD);
	int pxw = lw * TILE_SIZE, pxh = lh * TILE_SIZE;

	GameLevelState* newlvl = new GameLevelState(roomid, DNGN_MAIN_BRANCH,
			roomid, pxw, pxh, wandering);

	GameLevelState* prevlvl = gs->get_level(); //Save level context
	gs->set_level(newlvl); //Set level context to new level

	printf("level.init RNG state at %d numbers\n",
			mt.DEBUG_INFO_amnt_generated);
	for (int i = 0; i < roomsettings.size(); i++) {
		generate_rooms(roomsettings[i], mt, level);
	}
	printf("ROOMS state at %d numbers\n", mt.DEBUG_INFO_amnt_generated);
	generate_tunnels(layout.tunnels, mt, level);
	printf("TUNNELS state at %d numbers\n", mt.DEBUG_INFO_amnt_generated);
	generate_features(content.features, mt, level, gs);
	printf("FEATURES state at %d numbers\n", mt.DEBUG_INFO_amnt_generated);
	generate_enemies(content.enemies, mt, level, gs);
	printf("ENEMIES state at %d numbers\n", mt.DEBUG_INFO_amnt_generated);
	generate_items(content.items, mt, level, gs);
	printf("ITEMS state at %d numbers\n", mt.DEBUG_INFO_amnt_generated);
	newlvl->rooms = level.rooms();

	gs->set_level(prevlvl); //Restore level context

	return newlvl;
}
