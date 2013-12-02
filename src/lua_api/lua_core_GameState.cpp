/*
 * lua_misc.cpp:
 *  Misc. functions that have limited general usage
 */

#include <ldraw/lua_ldraw.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>

#include <SDL.h>

#include "data/game_data.h"
#include "gamestate/GameState.h"
#include "gamestate/ScoreBoard.h"

#include "lua_api.h"

static void game_save(LuaStackValue filename) {
	FILE* file = fopen(filename.as<const char*>(), "wb");
	SerializeBuffer sb(file, SerializeBuffer::OUTPUT);
	lua_api::gamestate(filename)->serialize(sb);
	sb.flush();
	fclose(file);
}

static int game_score_board_store(lua_State* L) {
	score_board_store(lua_api::gamestate(L),
			lua_gettop(L) >= 1 ? lua_toboolean(L, 1) : false);
	return 0;
}

static void game_load(LuaStackValue filename) {
	FILE* file = fopen(filename.as<const char*>(), "rb");
	SerializeBuffer sb(file, SerializeBuffer::INPUT);
	lua_api::gamestate(filename)->deserialize(sb);
	fclose(file);
}

static int game_resources_load(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	init_game_data(gs->game_settings(), gs->luastate());
	return 0;
}

static int game_step(lua_State* L) {
	bool status = lua_api::gamestate(L)->step();
	lua_pushboolean(L, status); // should quit on false
	return 1;
}

static int game_draw(lua_State* L) {
	lua_api::gamestate(L)->draw();
	return 0;
}

static int game_input_capture(lua_State* L) {
	bool reset_previous = lua_gettop(L) <= 0 || lua_toboolean(L, 1);
	bool trigger_events = lua_gettop(L) <= 1 || lua_toboolean(L, 2);
	bool status = lua_api::gamestate(L)->update_iostate(reset_previous, trigger_events);
	lua_pushboolean(L, status); // should quit on false
	return 1;
}

static int game_input_handle(lua_State* L) {
	bool status = lua_api::gamestate(L)->pre_step();
	lua_pushboolean(L, status); // should quit on false
	return 1;
}

LuaValue luaapi_settings_proxy(lua_State* L) {
	LuaValue meta = luameta_new(L, "Settings");
	LuaValue getters = luameta_getters(meta);
	LuaValue setters = luameta_setters(meta);

#define BIND(x) \
	luawrap::bind_getter( getters[#x], &GameSettings::x); \
	luawrap::bind_setter( setters[#x], &GameSettings::x)

	BIND(font);
	BIND(menu_font);
	BIND(fullscreen);
	BIND(regen_on_death);
	BIND(view_width);
	BIND(view_height);
	BIND(ip);
	BIND(port);
	BIND(lobby_server_url);
	BIND(steps_per_draw);
	BIND(frame_action_repeat);
	BIND(free_memory_while_idle);
	BIND(invincible);
	BIND(time_per_step);
	BIND(draw_diagnostics);
	BIND(username);
	BIND(class_type);
	BIND(network_debug_mode);
	BIND(savereplay_file);
	BIND(loadreplay_file);
	BIND(verbose_output);
	BIND(autouse_health_potions);
	BIND(autouse_mana_potions);
	BIND(keep_event_log);
	BIND(comparison_event_log);
#undef BIND

	// We must ensure that the enum type is looked up properly
	// This is not ideal but better than accidentally not casting to int or writing more boilerplate
	luawrap::install_casted_type<GameSettings::connection_type, int>();
	luawrap::bind_getter(getters["connection_type"], &GameSettings::conntype);
	luawrap::bind_setter(setters["connection_type"], &GameSettings::conntype);

	return meta;
}

static void register_gamesettings(lua_State* L) {
	luawrap::install_userdata_type<GameSettings*, luaapi_settings_proxy>();

	luawrap::globals(L)["settings"] = &lua_api::gamestate(L)->game_settings();
}

static int game_frame(lua_State* L) {
	lua_pushnumber(L, lua_api::gamestate(L)->frame());
	return 1;
}

static long garbage_collect_while_waiting(lua_State* L, long wait_micro) {
	// How many times do we step garbage collection before checking time ?
	static const int ITERS_PER_CHECK = 2;

	// Try to do 4ms of garbage collection for every 6 frames
	static const int MICROS_PER_FRAME_GROUP = 4000 /*4 ms*/;
	static const int FRAME_PER_GROUP = 6;

	static int n_frames = 0;
	static long time_spent_micro = 0;

	Timer timer;

	n_frames++;

	if (time_spent_micro >= MICROS_PER_FRAME_GROUP) {
		if (n_frames > FRAME_PER_GROUP) {
			time_spent_micro = 0;
			n_frames = 0;
		} else {
			return 0;
		}
	}

	wait_micro = std::min(wait_micro, MICROS_PER_FRAME_GROUP - time_spent_micro);

	int initial_memusage = lua_gc(L, LUA_GCCOUNT,0);
	while (timer.get_microseconds() < wait_micro) {
		for (int i = 0; i < ITERS_PER_CHECK; i++){
			if (lua_gc(L, LUA_GCSTEP, 0)) {
				goto label_AfterLoop;
			}
		}
	}

label_AfterLoop:
	time_spent_micro += timer.get_microseconds();
	int mem_drop = (initial_memusage - lua_gc(L, LUA_GCCOUNT,0));

	return timer.get_microseconds();
}

static void lapi_wait(LuaStackValue wait_time) {
	lua_State* L = wait_time.luastate();
	long wait_micro = wait_time.as<double>() * 1000;

	GameState* gs = lua_api::gamestate(wait_time);
	GameSettings& settings = gs->game_settings();
	if (settings.free_memory_while_idle) {
		wait_micro -= garbage_collect_while_waiting(L, wait_micro);
	}

	long remaining_wait_ms = wait_micro / 1000;

	if (remaining_wait_ms > 0) {
		SDL_Delay(remaining_wait_ms);
	}
}

namespace lua_api {

	static void register_game_getters(lua_State* L, LuaValue& game) {
		LuaValue metatable = luameta_new(L, "game table");
		LuaValue getters = luameta_getters(metatable);

		getters["frame"].bind_function(game_frame);

		game.push();
		metatable.push();
		lua_setmetatable(L, -2);
		lua_pop(L, 1);
	}

	void register_lua_core_GameState(lua_State* L) {
		// Install ScoreBoardEntry so we can bind fetch_scores directly
		luawrap::install_type<ScoreBoardEntry, ScoreBoardEntry::lua_push>();

		register_gamesettings(L);

		LuaValue globals = luawrap::globals(L);
		LuaValue game = register_lua_submodule(L, "core.GameState");

		game["resources_load"].bind_function(game_resources_load);

		game["save"].bind_function(game_save);
		game["load"].bind_function(game_load);

		game["step"].bind_function(game_step);
		game["draw"].bind_function(game_draw);
		game["wait"].bind_function(lapi_wait);

		game["input_capture"].bind_function(game_input_capture);
		game["input_handle"].bind_function(game_input_handle);

		game["score_board_fetch"].bind_function(score_board_fetch);
		game["score_board_store"].bind_function(game_score_board_store);

		register_game_getters(L, game);
	}
}
