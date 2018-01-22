/*
 * lua_misc.cpp:
 *  Misc. functions that have limited general usage
 */

#include <ldraw/lua_ldraw.h>
#include <ldraw/display.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>
#include <fstream>

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

static int game_mark_loading(lua_State* L) {
    lua_api::gamestate(L)->is_loading_save() = true;
    return 0;
}
static void game_load(LuaStackValue filename) {
        // std::ifstream file(filename, std::ios::binary | std::ios::ate);
        // std::streamsize size = file.tellg();
        // file.seekg(0, std::ios::beg);
        // std::vector<char> buffer(size);
        // file.read(buffer.data(), size);
	FILE* file = fopen(filename.as<const char*>(), "rb");
	SerializeBuffer sb(file, SerializeBuffer::INPUT);
	lua_api::gamestate(filename)->deserialize(sb);
	// Ensure game state is set to 'loading'; this signals that the game state should not be started anew
	lua_api::gamestate(filename)->is_loading_save() = true;
	fclose(file);
}

// Call on main thread
static int game_main_call(lua_State* L) {
	// Convert to main thread:
	auto* gs = lua_api::gamestate(L);
	auto* mainL = gs->luastate();
    int initial_mainL_top = lua_gettop(mainL);
	int n_args = lua_gettop(L);
	if (L == mainL) {
		// Already on main thread? Allow through.
		lua_call(L, n_args - 1, LUA_MULTRET);
		return lua_gettop(L);
	}
	// Otherwise, we funnel over arguments:
	for (int i = 1; i <= n_args; i++) {
		// Write from L
		lua_pushnumber(L, 10);
		lua_pushvalue(L, i);
		lua_rawset(L, LUA_REGISTRYINDEX);

		// Read from mainL
        lua_pushnumber(mainL, 10);
		lua_rawget(mainL, LUA_REGISTRYINDEX);
	}

	lua_call(mainL, n_args - 1, 1);

	// Write from mainL
    lua_pushnumber(mainL, 10);
	lua_pushvalue(mainL, -2);
	lua_rawset(mainL, LUA_REGISTRYINDEX);

	// Read from L
    lua_pushnumber(L, 10);;
	lua_rawget(L, LUA_REGISTRYINDEX);

    lua_settop(mainL, initial_mainL_top);
	// Return read value
	return 1;
}

static int game_resources_load(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	init_resource_data(gs->luastate());
	return 0;
}

static int game_step(lua_State* L) {
	bool status = lua_api::gamestate(L)->step();
	lua_pushboolean(L, status); // should quit on false
	return 1;
}

// Log events directly, without doing a formatting pass. 
// Used with event_log_is_active in Lua.
static void game_raw_event_log(const char* str) {
    event_log("%s", str);
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

static void game_loop(LuaValue draw_func) {
	int frames = 0;
        lua_State* L = draw_func.luastate();
	while (1) {
		frames += 1;
		SDL_Event event;
	        if (!lua_api::gamestate(L)->update_iostate(true, false)) {
                    return; // Exit game loop
                }
		ldraw::display_draw_start();
                draw_func.push();
                lua_pushnumber(L, frames);
                lua_call(L, 1, 1);
                bool should_exit = lua_toboolean(L, -1);
                lua_pop(L, 1);
		ldraw::display_draw_finish();
                if (should_exit) {
                    return; // Exit game loop
                }
		SDL_Delay(5);
	}
}

static int game_input_clear(lua_State* L) {
    IOController& controller = lua_api::gamestate(L)->io_controller();
    controller.clear();
    return 0;
}

static void game_simulate_key_press(LuaStackValue key) {
    IOController& controller = lua_api::gamestate(key)->io_controller();
    controller.set_key_press_state(key.to_num());
    controller.set_key_down_state(key.to_num());
}

static void game_for_screens(LuaStackValue func) {
	lua_api::gamestate(func)->for_screens([&]() {
		func.push();
		lua_call(func.luastate(),  0,0);
	});
}

static void game_screen_set(LuaStackValue screen) {
	lua_api::gamestate(screen)->screens.set_screen(screen.to_int());
}

static int game_screen_amount(LuaStackValue screen) {
	return lua_api::gamestate(screen)->screens.amount();
}

static int game_screen_get(LuaStackValue screen) {
	return lua_api::gamestate(screen)->screens.amount();
}

static int game_input_handle(lua_State* L) {
	bool status = lua_api::gamestate(L)->pre_step(luawrap::get_defaulted(L, 1, true));
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

static int game_lazy_reset(lua_State* L) {
	lua_api::gamestate(L)->game_world().lazy_reset();
	return 0;
}

static int game_reset(lua_State* L) {
	lua_api::gamestate(L)->game_world().reset();
	return 0;
}

static int game_clear_players(lua_State* L) {
	//const std::string& name, PlayerInst* player,
	//const std::string& classtype, bool is_local_player, int net_id
	//
	lua_api::gamestate(L)->player_data().clear();
	return 0;
}
static void game_register_player(LuaStackValue name, const char* classtype, LuaValue input_source, bool is_local_player, int net_id) {
	//const std::string& name, PlayerInst* player,
	//const std::string& classtype, bool is_local_player, int net_id
	//
	lua_api::gamestate(name)->player_data().register_player(
			name.to_str(), nullptr,
			classtype, input_source,
			is_local_player, net_id);
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
		game["main_call"].bind_function(game_main_call);

		game["mark_loading"].bind_function(game_mark_loading);
		game["step"].bind_function(game_step);
		game["draw"].bind_function(game_draw);
		game["raw_event_log"].bind_function(game_raw_event_log);
		game["for_screens"].bind_function(game_for_screens);
                game["reset"].bind_function(game_reset);
                game["lazy_reset"].bind_function(game_lazy_reset);
		game["screen_set"].bind_function(game_screen_set);
		game["screen_get"].bind_function(game_screen_get);
		game["screen_amount"].bind_function(game_screen_amount);
                // Can directly bind event_log_is_active:
		game["event_log_is_active"].bind_function(event_log_is_active);
                game["wait"].bind_function(lapi_wait);

		game["input_capture"].bind_function(game_input_capture);
		game["input_handle"].bind_function(game_input_handle);
		game["game_loop"].bind_function(game_loop);

		game["_input_clear"].bind_function(game_input_clear);
		game["_simulate_key_press"].bind_function(game_simulate_key_press);

		game["score_board_fetch"].bind_function(score_board_fetch);
		game["score_board_store"].bind_function(game_score_board_store);
		game["clear_players"].bind_function(game_clear_players);
		game["register_player"].bind_function(game_register_player);

		register_game_getters(L, game);
	}
}
