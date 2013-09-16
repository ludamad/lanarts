/*
 * lua_newapi.cpp:
 *  New API, rename to lua_api once completed
 */

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/luameta.h>

#include <lcommon/lua_utils.h>
#include <lcommon/lua_lcommon.h>

#include <ldraw/lua_ldraw.h>

#include <ldungeon_gen/lua_ldungeon.h>

#include "gamestate/GameState.h"

#include "lua_api/lua_yaml.h"
#include "lua_api/lua_newapi.h"
#include "lua_api/lua_gameinst.h"

extern "C" {
#include "luasocket/luasocket.h"
#include "luasocket/mime.h"
}

#include "lua_newapi.h"

// NB: the -address- of this key is used, not the actual string.
static char GAMESTATE_KEY[] = "";

namespace lua_api {

	void luacall_game_won(lua_State* L) {
		luawrap::globals(L)["Engine"]["game_won"].push();
		luawrap::call<void>(L);
	}

	void event_player_death(lua_State* L, PlayerInst* player) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		luawrap::call<void>(L, "PlayerDeath", (GameInst*) player);
	}

	void event_player_init(lua_State* L, PlayerInst* player) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		luawrap::call<void>(L, "PlayerInit", (GameInst*) player);
	}

	void event_monster_init(lua_State* L, EnemyInst* enemy) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		luawrap::call<void>(L, "MonsterInit", (GameInst*) enemy);
	}

	void event_monster_death(lua_State* L, EnemyInst* enemy) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		luawrap::call<void>(L, "MonsterDeath", (GameInst*) enemy);
	}

	void add_search_path(lua_State* L, const char* path) {
		LuaValue package = luawrap::globals(L)["package"];
		package["path"].push();
		lua_pushfstring(L, "%s;%s", lua_tostring(L, -1), path);
		package["path"].pop();
		lua_pop(L, 1);
	}

	void require(lua_State* L, const char* path) {
		lua_getglobal(L, "require");
		lua_pushstring(L, path);
		lua_call(L, 1, 0);
	}


	static int safe_dofile(LuaStackValue filename) {
		luawrap::dofile(filename.luastate(), filename.to_str());
		return 0;
	}

	/* Simple lua configuration.
	 * Lanarts assumed the lua state was created with this function. */
	lua_State* create_configured_luastate() {
		// TODO: Remove all implicit yaml<->lua
		luawrap::install_type<YAML::Node, lua_pushyaml>();

		lua_State* L = lua_open();
		luaL_openlibs(L);

		// configure_lua_packages:
		LuaValue package = luawrap::globals(L)["package"];
		package["loadlib"].set_nil(); // Remove until library loading plan is made
		package["path"] = "./?.lua";

		LuaSpecialValue globals = luawrap::globals(L);
		globals["dofile"].bind_function(safe_dofile);
		return L;
	}

	// Register all the lanarts API functions and types
	static void register_gamestate(GameState* gs, lua_State* L) {
		lua_pushlightuserdata(L, (void*)(GAMESTATE_KEY));
		lua_pushlightuserdata(L, gs);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	// Grab the GameState back-pointer
	// Used to implement lua_api functions that affect GameState
	GameState* gamestate(lua_State* L) {
		lua_pushlightuserdata(L, (void*)(GAMESTATE_KEY));
		lua_gettable(L, LUA_REGISTRYINDEX);
		GameState* gs = (GameState*)(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return gs;
	}


	// Convenience function that performs above on captured lua state
	GameState* gamestate(const LuaStackValue& val) {
		return gamestate(val.luastate());
	}

	void register_lua_libraries(lua_State* L) {
		LuaField preload = luawrap::globals(L)["package"]["preload"];
		lua_pushcfunction(L, luaopen_socket_core);
		lua_api::register_lua_submodule_loader(L, "core.socket.core", LuaValue::pop_value(L));
		lua_pushcfunction(L, luaopen_mime_core);
		lua_api::register_lua_submodule_loader(L, "core.mime.core", LuaValue::pop_value(L));

		lua_register_lcommon(L);
		preload["socket.core"].bind_function(luaopen_socket_core);
		preload["mime.core"].bind_function(luaopen_mime_core);
	}

	void register_lua_core_GameMap(lua_State* L);
	void register_lua_core_CollisionAvoidance(lua_State* L);
	void register_lua_core_PathFinding(lua_State* L);

	static int lua_lanarts_panic(lua_State* L) {
		luawrap::errorfunc(L);
		throw std::runtime_error(format("LUA PANIC: %s", lua_tostring(L, -1)));
		return 0;
	}

	// Register all the lanarts API functions and types
	void register_api(GameState* gs, lua_State* L) {
		// May be required by loading that follows:
		register_lua_libraries(L);

		LuaValue globals = luawrap::globals(L);
		LuaValue game = lua_ensure_protected_table(globals["Game"]);
		// Holds engine hooks
		LuaValue engine = luawrap::ensure_table(globals["Engine"]);

		register_gamestate(gs, L);
		register_general_api(L);

		LuaValue map_gen = register_lua_submodule(L, "core.MapGeneration");
		ldungeon_gen::lua_register_ldungeon(map_gen, &gs->rng(), false);

		lua_atpanic(L, lua_lanarts_panic);

		// Old-style API
		register_io_api(L);
		register_net_api(L);
		register_gamestate_api(L);
		register_gameworld_api(L);
		register_event_log_api(L);
		register_display_api(L);
		register_tiles_api(L);

		lua_spelltarget_bindings(L);
		lua_effectivestats_bindings(gs, L);
		lua_combatstats_bindings(gs, L);

		// New-style API
		register_lua_core_GameMap(L);
		register_lua_core_CollisionAvoidance(L);
		register_lua_core_PathFinding(L);
	}

	void luacall_post_draw(lua_State* L) {
		luawrap::globals(L)["Engine"].push();
		lua_getfield(L, -1, "post_draw");
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
		} else {
			luawrap::call<void>(L);
		}
	}

	void luacall_overlay_draw(lua_State* L) {
		luawrap::globals(L)["Engine"].push();
		lua_getfield(L, -1, "overlay_draw");
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
		} else {
			luawrap::call<void>(L);
		}
	}


	bool luacall_handle_event(lua_State* L, SDL_Event* e) {
		bool handled = false;

		luawrap::globals(L)["Engine"].push();
		LuaStackValue(L, -1)["io_event_occurred"].push();

		if (!lua_isnil(L, -1)) {
			luawrap::push(L, e);
			lua_call(L, 1, 1);
			handled = lua_toboolean(L, -1);
		}

		lua_pop(L, 2); // pop game table & function/result

		return handled;
	}


	void luacall_hitsound(lua_State* L) {
		luawrap::globals(L)["play_hit_sound"].push();
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
		} else {
			luawrap::call<void>(L);
		}
	}
}
