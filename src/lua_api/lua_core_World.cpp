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

#include "lua_api.h"

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
		return game_class_data.get(_entry(proxy).classtype).name.c_str();
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

// TODO obsolete the above
static int world_players(lua_State* L) {
        LuaValue value(L);
	value.newtable();
        int i = 1;
	for (PlayerDataEntry& entry : lua_api::gamestate(L)->player_data().all_players()) {
            value[i].newtable();
            value[i]["name"] = entry.player_name;
            value[i]["instance"] = entry.player();
            value[i]["class_name"] = entry.classtype;
            i += 1;
	}
        value.push();
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
	GameMapState* map = gs->game_world().get_level(level_id["_id"].to_int());
	gs->game_world().spawn_players(map, positions);
}

namespace lua_api {
	void register_lua_core_GameWorld(lua_State* L) {
		LuaValue world = register_lua_submodule(L, "core.World");
		luawrap::install_userdata_type<PlayerDataProxy,
				PlayerDataProxy::metatable>();

		LuaValue metatable = luameta_new(L, "World");
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
}
