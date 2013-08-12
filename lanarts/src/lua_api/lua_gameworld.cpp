/*
 * lua_gameworld.cpp:
 *  Query the state of the game world.
 */

#include <SDL.h>

#include <ldungeon_gen/Map.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"
#include "gamestate/GameSettings.h"

#include "draw/TileEntry.h"

#include "stats/ClassEntry.h"

#include "lua_newapi.h"

#include "lua_api/lua_api.h"
#include "lua_api/lua_gameinst.h"

// Keep all documentation in doc/level.luadoc

// A bit of a hack, but its OK and at least self-contained:
struct PlayerDataProxy {

	int index;
	PlayerDataProxy(int index = 0) :
			index(index) {
	}

	static PlayerDataEntry& _entry(const LuaStackValue& proxy) {
		PlayerDataProxy* pdp = proxy.as<PlayerDataProxy*>();
		std::vector<PlayerDataEntry>& players =
				lua_api::gamestate(proxy)->player_data().all_players();
		return players.at(pdp->index);
	}

	static const char* name(const LuaStackValue& proxy) {
		return _entry(proxy).player_name.c_str();
	}

	static const char* class_name(const LuaStackValue& proxy) {
		return game_class_data.at(_entry(proxy).classtype).name.c_str();
	}

	static GameInst* instance(const LuaStackValue& proxy) {
		return (GameInst*) _entry(proxy).player();
	}

	static LuaValue metatable(lua_State* L) {
		LuaValue meta = luameta_new(L, "PlayerData");
		LuaValue getters = luameta_getters(meta);

		getters["name"].bind_function(PlayerDataProxy::name);
		getters["instance"].bind_function(PlayerDataProxy::instance);
		getters["class_name"].bind_function(PlayerDataProxy::class_name);

		return meta;
	}
};


static Tile resolve_tile(ldungeon_gen::Map& map, MTwist& mt, Pos& xy) {
	using namespace ldungeon_gen;
	int tileid, subtileid;

	TileEntry& entry = res::tile(tileid);
	TileLayoutRules& rules = entry.layout_rules;

	/* Simple random tile */
	if (rules.orientations.empty()) {
		subtileid = mt.rand(rules.rest);
	} else {
		/* Oriented tile. */
		// Check if the tiles in the surrounding area are the same as this one.
		bool same_tile[3][3];

	}
	return Tile(tileid, subtileid);
}

static int world_players(lua_State* L) {
	int nplayers = lua_api::gamestate(L)->player_data().all_players().size();
	lua_newtable(L);
	for (int i = 0; i < nplayers; i++) {
		luawrap::push<PlayerDataProxy>(L, PlayerDataProxy(i));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}
static int world_player_amount(lua_State* L) {
	int nplayers = lua_api::gamestate(L)->player_data().all_players().size();
	lua_pushinteger(L, nplayers);
	return 1;
}

// game world functions
static int world_local_player(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, (GameInst*) gs->local_player());
	return 1;
}

static void world_players_spawn(LuaStackValue level_id, const std::vector<Pos>& positions) {
	GameState* gs = lua_api::gamestate(level_id);
	GameMapState* map = gs->game_world().get_level(level_id.to_int());
	gs->game_world().spawn_players(map, positions);
}

namespace lua_api {
	static void register_gameworld_submodule(lua_State* L,
			const LuaValue& world) {
		luawrap::install_userdata_type<PlayerDataProxy,
				PlayerDataProxy::metatable>();

		LuaValue metatable = luameta_new(L, "GameWorld");
		LuaValue getters = luameta_getters(metatable);
		LuaValue functions = luameta_constants(metatable);

		functions["players_spawn"].bind_function(world_players_spawn);

		getters["players"].bind_function(world_players);
		getters["player_amount"].bind_function(world_player_amount);
		getters["local_player"].bind_function(world_local_player);

		world.push();
		metatable.push();
		lua_setmetatable(L, -2);
		lua_pop(L, 1);
	}

	void register_gameworld_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		lua_register_gameinst(L);

		LuaValue world = register_lua_submodule(L, "core.GameWorld");
		register_gameworld_submodule(L, world);
	}
}
