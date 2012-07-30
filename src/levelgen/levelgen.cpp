/*
 * levelgen.cpp:
 *  Contains procedures related to generating a full dungeon level
 */

extern "C" {
#include <lua/lua.h>
}

#include <cstdio>

#include "dungeon_data.h"

#include "../objects/player/PlayerInst.h"

#include "../gamestate/GameState.h"

#include "lua_levelgen_funcs.h"
#include "GeneratedLevel.h"
#include "shapefill.h"
#include "levelgen.h"

const int TOO_MANY_FAILURES = 100;

//return false on failure
bool generate_room_at(MTwist& mt, GeneratedLevel& level, const Region& r,
		int padding, int mark) {
	if (!level.verify(r))
		return false;
	Sqr val = Sqr(true, false, false, (feature_t)mark, 0,
			level.rooms().size() + 1);
	level.set_region_with_perimeter(r, val, padding);

	Room room(
			Region(r.x + padding, r.y + padding, r.w - padding * 2,
					r.h - padding * 2), 0);
	level.rooms().push_back(room);

	return true;
}

bool generate_room(MTwist& mt, GeneratedLevel& level, int rw, int rh,
		int padding, int mark, int max_attempts) {

	int levelw = level.width(), levelh = level.height();
	for (int attempts = 0; attempts < max_attempts; attempts++) {
		int rx = mt.rand(1, levelw - rw - padding) | 1;
		int ry = mt.rand(1, levelh - rh - padding) | 1;

		Region r(rx, ry, rw, rh);

		if (generate_room_at(mt, level, r, padding, mark)) {
			return true;
		}
	}
	return false;
}

static bool generate_room_lua(lua_State* L, GeneratedLevel& level, int rw,
		int rh, int padding, int max_attempts) {

	lua_getfield(L, LUA_GLOBALSINDEX, "gen_room");
	lua_push_generatedlevel(L, level);
	lua_pushnumber(L, padding);
	lua_pushnumber(L, rw);
	lua_pushnumber(L, rh);
	lua_pushnumber(L, max_attempts);
	lua_call(L, 5, 1);
	bool ret = !lua_isnil(L, -1);
	lua_pop(L, 1);
	return ret;
}

void generate_rooms(lua_State* L, const RoomGenSettings& rs, MTwist& mt,
		GeneratedLevel& level) {
	int pad = rs.room_padding;
	int nrooms = mt.rand(rs.amount_of_rooms);
	Range sizerange(rs.size.min + pad * 2, rs.size.max + pad * 2);

	int failures = 0;
	for (int i = 0; i < nrooms; i++) {
		bool marked = (mt.rand(4) == 0);
		int mark = marked ? mt.rand(1, 4) : 0;
		for (;; failures++) {

			int rw = mt.rand(sizerange), rh = mt.rand(sizerange);

			if (generate_room(mt, level, rw, rh, rs.room_padding, mark, 20)) {
				break;
			}
//			if (generate_room_lua(L, level, rw, rh, rs.room_padding, 20)) {
//				break;
//			}
			if (failures > TOO_MANY_FAILURES)
				goto NoMoreRooms;
			// Goto below
		}
	}
	NoMoreRooms: // Come from above

	std::vector<Room>& room_list = level.rooms();
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

Pos get_level_offset(GameState* gs, int lw, int lh) {
	GameTiles& tiles = gs->tile_grid();
	int tw = tiles.tile_width(), th = tiles.tile_height();

	return Pos((tw - lw) / 2, (th - lh) / 2);
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

	lua_State* L = gs->get_luastate();

	int LEVEL_BORDER_PAD = 10;
	int lw = std::min(128, w + LEVEL_BORDER_PAD), lh = std::min(128,
			h + LEVEL_BORDER_PAD);
	int pxw = lw * TILE_SIZE, pxh = lh * TILE_SIZE;

	GameLevelState* newlvl = new GameLevelState(roomid, pxw, pxh, wandering);

	GameLevelState* prevlvl = gs->get_level(); //Save level context
	gs->set_level(newlvl); //Set level context to new level

	level.initialize(w, h, get_level_offset(gs, w, h), layout.solid_fill);

	printf("level.init at %d RNG calls\n", mt.DEBUG_INFO_amnt_generated);
	for (int i = 0; i < roomsettings.size(); i++) {
		generate_rooms(L, roomsettings[i], mt, level);
	}
	printf("ROOMS state at %d RNG calls\n", mt.DEBUG_INFO_amnt_generated);
	generate_tunnels(layout.tunnels, mt, level);
	printf("TUNNELS state at %d RNG calls\n", mt.DEBUG_INFO_amnt_generated);
	generate_features(content.features, mt, level, gs);
	printf("FEATURES state at %d RNG calls\n", mt.DEBUG_INFO_amnt_generated);
	generate_enemies(content.enemies, mt, level, gs);
	printf("ENEMIES state at %d RNG calls\n", mt.DEBUG_INFO_amnt_generated);
	generate_items(content.items, mt, level, gs);
	printf("ITEMS state at %d RNG calls\n", mt.DEBUG_INFO_amnt_generated);
	newlvl->rooms = level.rooms();

	for (int i = 0; i < level.instances().size(); i++) {
		gs->add_instance(level.instances()[i].get_instance());
	}

	gs->set_level(prevlvl); //Restore level context

	return newlvl;
}
