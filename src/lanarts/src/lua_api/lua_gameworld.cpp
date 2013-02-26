/*
 * lua_gameworld.cpp:
 *  Query the state of the game world.
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include <SDL.h>

#include "gamestate/GameState.h"
#include "gamestate/GameLevelState.h"
#include "gamestate/GameSettings.h"

#include "stats/ClassEntry.h"

#include "lua_newapi.h"

#include "lua_api/lua_api.h"

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

static int world_players(lua_State* L) {
	int nplayers = lua_api::gamestate(L)->player_data().all_players().size();
	lua_newtable(L);
	for (int i = 0; i < nplayers; i++) {
		luawrap::push<PlayerDataProxy>(L, PlayerDataProxy(i));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

// game world functions
static int world_local_player(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, (GameInst*) gs->local_player());
	return 1;
}

// level functions

static int level_regenerate(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int levelid = gs->get_level()->id();

	if (lua_gettop(L) > 0) {
		levelid = luaL_checknumber(L, 1);
	}

	gs->game_world().regen_level(levelid);
	return 0;
}

static int level_objects_list(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInstSet& insts = gs->get_level()->game_inst_set();
	luawrap::push(L, insts.to_vector());
	return 1;
}

static int level_objects(lua_State* L) {
	lua_pushcfunction(L, lua_api::l_itervalues);
	level_objects_list(L);
	lua_call(L, 1, 1);
	return 1;
}

static int level_monsters_list(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	const std::vector<obj_id>& mons = gs->monster_controller().monster_ids();
	lua_newtable(L);
	for (int i = 0; i < mons.size(); i++) {
		luawrap::push(L, gs->get_instance(mons[i]));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int level_monsters(lua_State* L) {
	lua_pushcfunction(L, lua_api::l_itervalues);
	level_monsters_list(L);
	lua_call(L, 1, 1);
	return 1;
}

// Look up a specific instance given an id
static int level_instance(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->get_instance(luaL_checkinteger(L, 1)));
	return 1;
}

// TODO add test for specific player
static bool level_object_visible(const LuaStackValue& obj) {
	GameState* gs = lua_api::gamestate(obj);
	return gs->object_visible_test(obj.as<GameInst*>());
}

static int level_object_place_free(lua_State* L) {
	int nargs = lua_gettop(L);
	GameState* gs = lua_api::gamestate(L);
	GameInst* inst = luawrap::get<GameInst*>(L, 1);
	Pos p = inst->pos();

	if (nargs > 1) {
		p = luawrap::get<Pos>(L, 2);
	}

	lua_pushboolean(L, !gs->solid_test(inst, p.x, p.y));
	return 1;
}

static bool level_place_free(const LuaStackValue& pos) {
	GameState* gs = lua_api::gamestate(pos);
	Pos p = pos.as<Pos>();
	return !gs->solid_test(NULL, p.x, p.y, 1);
}

static bool level_radius_place_free(const LuaStackValue& radius,
		const LuaStackValue& pos) {
	GameState* gs = lua_api::gamestate(radius);
	Pos p = pos.as<Pos>();
	return !gs->solid_test(NULL, p.x, p.y, radius.as<int>());
}

bool temporary_isgameinst(lua_State* L, int idx) {
	return lua_isuserdata(L, idx);
}

//TODO Figure out how other levels should be queried

namespace lua_api {
	static void register_gameworld_getters(lua_State* L,
			const LuaValue& world) {
		luawrap::install_userdata_type<PlayerDataProxy,
				PlayerDataProxy::metatable>();

		LuaValue metatable = luameta_new(L, "world table");
		LuaValue getters = luameta_getters(metatable);

		getters["players"].bind_function(world_players);
		getters["local_player"].bind_function(world_local_player);

		world.push();
		metatable.push();
		lua_setmetatable(L, -2);
		lua_pop(L, 1);
	}

	void register_gameworld_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		luawrap::install_type<GameInst*, lua_push_gameinst, lua_gameinst_arg,
				temporary_isgameinst>();

		register_gameworld_getters(L, globals["world"].ensure_table());

		LuaValue level = globals["level"].ensure_table();

		// Debug/special-case-only functions:
		level["regenerate"].bind_function(level_regenerate);

		// Query functions:
		level["objects_list"].bind_function(level_objects_list);
		level["objects"].bind_function(level_objects);

		level["instance"].bind_function(level_instance);

		level["monsters_list"].bind_function(level_monsters_list);
		level["monsters"].bind_function(level_monsters);

		level["object_visible"].bind_function(level_object_visible);
		level["object_place_free"].bind_function(level_object_place_free);
		level["place_free"].bind_function(level_place_free);
		level["radius_place_free"].bind_function(level_radius_place_free);
	}
}
