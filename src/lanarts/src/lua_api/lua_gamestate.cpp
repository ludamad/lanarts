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

#include "lua_newapi.h"

static void game_save(LuaStackValue filename) {
	FILE* file = fopen(filename.as<const char*>(), "wb");
	SerializeBuffer sb = SerializeBuffer::file_writer(file);
	lua_api::gamestate(filename)->serialize(sb);
	sb.flush();
	fclose(file);
}

static void game_load(LuaStackValue filename) {
	FILE* file = fopen(filename.as<const char*>(), "rb");
	SerializeBuffer sb = SerializeBuffer::file_reader(file);
	lua_api::gamestate(filename)->deserialize(sb);
	fclose(file);
}

static int game_resources_load(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	ldraw::lua_register_ldraw(L, luawrap::globals(L));
	init_lua_data(gs, gs->luastate());
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
	bool reset_previous = lua_gettop(L) == 0 || lua_toboolean(L, 1);
	bool status = lua_api::gamestate(L)->update_iostate(reset_previous);
	lua_pushboolean(L, status); // should quit on false
	return 1;
}

static int game_input_handle(lua_State* L) {
	bool quit = lua_api::gamestate(L)->pre_step();
	lua_pushboolean(L, quit);
	return 1;
}

LuaValue luaapi_settings_proxy(lua_State* L) {
	LuaValue meta = luameta_new(L, "Settings");
	LuaValue getters = luameta_getters(meta);
	LuaValue setters = luameta_setters(meta);

#define BIND(x) getters[#x].bind_getter(&GameSettings::x); \
	setters[#x].bind_setter(&GameSettings::x)

	BIND(font);
	BIND(menu_font);
	BIND(fullscreen);
	BIND(regen_on_death);
	BIND(view_width);
	BIND(view_height);
	BIND(ip);
	BIND(port);
	BIND(steps_per_draw);
	BIND(frame_action_repeat);
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
	getters["connection_type"].bind_getter(&GameSettings::conntype);
	setters["connection_type"].bind_setter(&GameSettings::conntype);

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

// A bit of a hack, but its OK and at least self-contained:
struct PlayerDataProxy {

	int index;
	PlayerDataProxy(int index = 0) :
			index(index) {
	}

	static PlayerDataEntry& _entry(const LuaStackValue& proxy) {
		PlayerDataProxy* pdp = proxy.as<PlayerDataProxy*>();
		std::vector<PlayerDataEntry>& players = lua_api::gamestate(proxy)->player_data().all_players();
		return players.at(pdp->index);
	}

	static const char* name(const LuaStackValue& proxy) {
		return _entry(proxy).player_name.c_str();
	}

	static const char* class_name(const LuaStackValue& proxy) {
		return game_class_data.at(_entry(proxy).classtype).name.c_str();
	}

	static GameInst* instance(const LuaStackValue& proxy) {
		return (GameInst*)_entry(proxy).player();
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

static int game_players(lua_State* L) {
	int nplayers = lua_api::gamestate(L)->player_data().all_players().size();
	lua_newtable(L);
	for (int i = 0; i < nplayers; i++) {
		luawrap::push<PlayerDataProxy>(L, PlayerDataProxy(i));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

namespace lua_api {

	static void register_game_getters(lua_State* L, LuaValue& game) {
		LuaValue meta = luameta_new(L, "__game");
		LuaValue getters = luameta_getters(meta);

		getters["frame"].bind_function(game_frame);
		getters["players"].bind_function(game_players);

		game.push();
		meta.push();
		lua_setmetatable(L, -2);
		lua_pop(L, 1);
	}

	void register_gamestate_api(lua_State* L) {
		luawrap::install_userdata_type<PlayerDataProxy,
				PlayerDataProxy::metatable>();

		register_gamesettings(L);

		LuaValue globals = luawrap::globals(L);
		LuaValue game = globals["game"].ensure_table();

		game["resources_load"].bind_function(game_resources_load);

		game["save"].bind_function(game_save);
		game["load"].bind_function(game_load);

		game["step"].bind_function(game_step);
		game["draw"].bind_function(game_draw);
		game["wait"].bind_function(SDL_Delay);

		game["input_capture"].bind_function(game_input_capture);
		game["input_handle"].bind_function(game_input_handle);

		register_game_getters(L, game);
	}
}
