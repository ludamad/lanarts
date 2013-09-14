/*
 * lua_tiles.cpp:
 *  Bindings for creating tilesets.
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>

#include <lcommon/lua_utils.h>

#include <lsound/lua_lsound.h>

#include <SDL.h>

#include "gamestate/GameState.h"

#include "interface/TextField.h"

#include "lua_newapi.h"

#include "draw/TileEntry.h"


static int tile_create(LuaStackValue args) {
	lua_State* L = args.luastate();
	TileEntry entry;
	LuaField images = args["images"];
	if (!images.isnil()) {
		entry.images = images.as<ImageList>();
		entry.layout_rules.rest = Range(0, entry.images.size());
	} else {
		LuaField img_orients= args["image_orientations"];
		std::vector<ImageList> orients = img_orients["orientations"].as<std::vector<ImageList> >();
		if (orients.size() != 9) {
			lua_pushstring(L, "Expected 9 entries in image_orientations.orientations.");
			return lua_error(L);
		}
		for (int i = 0; i < orients.size(); i++) {
			ImageList& list = orients[i];
			int start_ind = entry.images.size();
			entry.images.insert(entry.images.end(), list.begin(), list.end());
			entry.layout_rules.orientations.push_back(
					Range(start_ind, start_ind + list.size()));
		}
	}
	game_tile_data.push_back(entry);
	return game_tile_data.size() - 1;
}

/* Transforms a dictionary of image lists to a dictionary of tiles */
static LuaValue tileset_create(LuaStackValue fields) {
	lua_State* L = fields.luastate();
	LuaValue tileset(L);
	tileset.newtable();

	fields.push();
	lua_pushnil(L); // Initial key
	while (lua_next(L, -2) != 0) {
		tileset[lua_tostring(L, -2)] = tile_create(LuaStackValue(L, -1));
		lua_pop(L, 1); // Pop value
	}
	lua_pop(L, 1); // Pop table

	lua_protect_table(tileset);
	return tileset;
}

namespace lua_api {
	void register_tiles_api(lua_State* L) {
		LuaValue data = lua_ensure_protected_table(luawrap::globals(L)["Data"]);
		data["tile_create"].bind_function(tile_create);
		data["tileset_create"].bind_function(tileset_create);
	}
}
