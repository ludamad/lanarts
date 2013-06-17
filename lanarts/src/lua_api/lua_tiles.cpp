/*
 * lua_tiles.cpp:
 *  Bindings for creating tilesets.
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>

#include <lsound/lua_lsound.h>

#include <SDL.h>

#include "gamestate/GameState.h"

#include "interface/TextField.h"

#include "lua_newapi.h"

#include "draw/TileEntry.h"

static int tile_create(LuaStackValue imagelist) {
	TileEntry entry;
	entry.images = imagelist.as<std::vector<ldraw::Image> >();
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
	while (lua_next(L, -2)) {
		tileset[lua_tostring(L, -2)] = tile_create(LuaStackValue(L, -1));
		lua_pop(L, 1); // Pop value
	}
	lua_pop(L, 1); // Pop table
	return tileset;
}

namespace lua_api {
	void register_tiles_api(lua_State* L) {
		LuaValue data = luawrap::ensure_table(luawrap::globals(L)["Data"]);
		data["tile_create"].bind_function(tile_create);
		data["tileset_create"].bind_function(tileset_create);
	}
}
