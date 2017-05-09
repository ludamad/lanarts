/*
 * lua_api.cpp:
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
#include "lua_api/lua_api.h"

extern "C" {
#include "luasocket/luasocket.h"
#include "luasocket/mime.h"
// yaml bindings so that lua can work with yaml files:
int luayaml_module(lua_State *L);
// Box2D bindings for future map generation work:
int luaopen_b2_vendor(lua_State* L);
// From dependency lua_enet, enet bindings for future net code refactoring:
int luaopen_enet(lua_State* L);
}

#include "lua_api.h"

// NB: the -address- of this key is used, not the actual string.
static char GAMESTATE_KEY[] = "";

namespace lua_api {

	void luacall_game_won(lua_State* L) {
		luawrap::globals(L)["Engine"]["game_won"].push();
		luawrap::call<void>(L);
	}

	bool event_player_death(lua_State* L, PlayerInst* player) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
                // Should the game end?
		return luawrap::call<bool>(L, "PlayerDeath", (GameInst*) player);
	}

	void event_player_init(lua_State* L, PlayerInst* player) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		if (lua_isnil(L, -1)) {
			return lua_pop(L, 1);
		}
		luawrap::call<void>(L, "PlayerInit", (GameInst*) player);
	}

	void event_monster_init(lua_State* L, EnemyInst* enemy) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		if (lua_isnil(L, -1)) {
			return lua_pop(L, 1);
		}
		luawrap::call<void>(L, "MonsterInit", (GameInst*) enemy);
	}

	void event_monster_death(lua_State* L, EnemyInst* enemy) {
		luawrap::globals(L)["Engine"]["event_occurred"].push();
		if (lua_isnil(L, -1)) {
			return lua_pop(L, 1);
		}
		luawrap::call<void>(L, "MonsterDeath", (GameInst*) enemy);
	}

	void add_search_path(lua_State* L, const char* path) {
		LuaValue package = luawrap::globals(L)["package"];
		package["path"].push();
		lua_pushfstring(L, "%s;%s", lua_tostring(L, -1), path);
		package["path"].pop();
		lua_pop(L, 1);
	}

	void require(lua_State* L, const char* path, bool keep_return) {
		lua_getglobal(L, "require");
		lua_pushstring(L, path);
		lua_call(L, 1, keep_return ? 1 : 0);
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
		package["path"] = "?.lua";

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
		lua_register_lcommon(L);
		LuaValue preload = luawrap::globals(L)["package"]["preload"];
		preload["yaml"].bind_function(luayaml_module);
		preload["enet"].bind_function(luaopen_enet);
//		preload["b2"].bind_function(luaopen_b2_vendor);
		preload["socket.core"].bind_function(luaopen_socket_core);
		preload["mime.core"].bind_function(luaopen_mime_core);
	}

	void register_lua_core_Display(lua_State* L);
	void register_lua_core_GameObject(lua_State* L);
	void register_lua_core_GameMap(lua_State* L);
	void register_lua_core_GameState(lua_State* L);
	void register_lua_core_GameWorld(lua_State* L);
	void register_lua_core_RVOWorld(lua_State* L);
	void register_lua_core_Bresenham(lua_State* L);
	void register_lua_core_PathFinding(lua_State* L);
	void register_lua_core_Keyboard(lua_State* L);
	void register_lua_core_Mouse(lua_State* L);
	void register_lua_core_MiscSpellAndItemEffects(lua_State* L);
	void register_lua_core_Serialization(lua_State* L);

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

		LuaValue map_gen = register_lua_submodule(L, "core.SourceMap");
		ldungeon_gen::lua_register_ldungeon(map_gen, false);

		lua_atpanic(L, lua_lanarts_panic);

		// Old-style API
		register_io_api(L);
		register_net_api(L);
		register_event_log_api(L);
		register_tiles_api(L);

		lua_spelltarget_bindings(L);
		lua_effectivestats_bindings(gs, L);
		lua_combatstats_bindings(gs, L);

		// New-style API
		register_lua_core_Display(L);
		register_lua_core_GameObject(L);
		register_lua_core_GameMap(L);
		register_lua_core_GameState(L);
		register_lua_core_GameWorld(L);
		register_lua_core_RVOWorld(L);
		register_lua_core_Bresenham(L);
		register_lua_core_PathFinding(L);
		register_lua_core_Keyboard(L);
        register_lua_core_MiscSpellAndItemEffects(L);
        register_lua_core_Mouse(L);
		register_lua_core_Serialization(L);
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
