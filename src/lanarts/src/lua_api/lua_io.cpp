/*
 * lua_io.cpp:
 *  Bindings for checking IO states
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

// Lets meet lua half-way, we accept LuaStackValue which is an efficient way
// of saying we take any lua value. This also lets us access our LuaState!

static int lua_tokeycode(const LuaStackValue& value) {
	lua_State* L = value.luastate();
	int idx = value.index();

	if (lua_type(L, idx) == LUA_TNUMBER) {
		return lua_tonumber(L, idx);
	}

	if (lua_isstring(L, idx)) {
		size_t size;
		const char* str = lua_tolstring(L, idx, &size);
		if (size != 1) {
			luaL_error(L,
					"Expected key number or one-character string for keycode, but got \"%s\".",
					str);
			return 0;
		}
		return tolower(str[0]);
	}

	luaL_error(L,
			"Expected key number or one-character string for keycode, but got %s.",
			lua_typename(L, idx));
	return 0;
}

static bool key_pressed(const LuaStackValue& value) {
	GameState* gs = lua_api::gamestate(value.luastate());
	return gs->key_press_state(lua_tokeycode(value));
}
static bool key_held(const LuaStackValue& value) {
	GameState* gs = lua_api::gamestate(value.luastate());
	return gs->key_down_state(lua_tokeycode(value));
}

static int mouse_xy(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->mouse_pos());
	return 1;
}

static int mouse_left_held(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->mouse_left_down());
	return 1;
}

static int mouse_left_pressed(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->mouse_left_click());
	return 1;
}
static int mouse_right_held(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->mouse_right_down());
	return 1;
}

static int mouse_right_pressed(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->mouse_right_click());
	return 1;
}

static Pos screen_coords(const LuaStackValue& value) {
	GameState* gs = lua_api::gamestate(value);
	return on_screen(gs, value.as<Pos>());
}

static Pos world_coords(const LuaStackValue& value) {
	GameState* gs = lua_api::gamestate(value);
	Pos p = value.as<Pos>();
	p.x += gs->view().x;
	p.y += gs->view().y;
	return p;
}

/* START of TextField bindings */

static void textfield_step(TextField& textfield) {
	textfield.step();
}

static void textfield_clear(TextField& textfield) {
	textfield.clear();
}

static void textfield_clear_keystate(TextField& textfield) {
	textfield.clear_keystate();
}

static bool textfield_handle_event(TextField& textfield, SDL_Event* event) {
	return textfield.handle_event(event);
}

static int lua_newtextinput(lua_State* L) {
	const char* text = lua_gettop(L) >= 2 ? lua_tostring(L, 2) : "";
	luawrap::push(L, TextField(lua_tointeger(L, 1), text));
	return 1;
}

LuaValue lua_textfieldmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "TextInput");

	LuaValue methods = luameta_constants(meta);
	LuaValue getters = luameta_getters(meta);
	LuaValue setters = luameta_setters(meta);

	getters["text"] = &luawrap::getter<TextField, const std::string&,
			&TextField::text>;
	getters["max_length"] = &luawrap::getter<TextField, int,
			&TextField::max_length>;
	setters["text"] = &luawrap::setter<TextField, const std::string&,
			&TextField::set_text>;

	methods["step"].bind_function(textfield_step);
	methods["event_handle"].bind_function(textfield_handle_event);
	methods["clear"].bind_function(textfield_clear);
	methods["clear_keystate"].bind_function(textfield_clear_keystate);

	luameta_gc<TextField>(meta);

	return meta;
}

static void register_textfield(lua_State* L) {
	LuaValue textfield = luawrap::globals(L)["TextInput"].ensure_table();
	textfield["create"].bind_function(lua_newtextinput);
	luawrap::install_userdata_type<TextField, &lua_textfieldmetatable>();
}

/* END of TextField bindings */

static int get_events(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	std::vector<SDL_Event>& events = gs->io_controller().get_events();
	luawrap::push(L, events);
	return 1;
}

static void register_input_table(lua_State* L) {
	LuaValue input = luawrap::globals(L)["input"].ensure_table();

	LuaValue meta = luameta_new(L, "<InputTable>");
	LuaValue getters = luameta_getters(meta);

	getters["events"].bind_function(get_events);

	input.push();
	meta.push();
	lua_setmetatable(L, -2);
	lua_pop(L, 1);
}

namespace lua_api {
	void register_io_api(lua_State* L) {
		luawrap::install_plaindata_type<SDL_Event>();

		lua_register_lsound(L);
		register_input_table(L);

		register_textfield(L);

		LuaValue globals = luawrap::globals(L);
		LuaValue globalgetters = lua_api::global_getters(L);

		globalgetters["mouse_xy"].bind_function(mouse_xy);
		globalgetters["mouse_left_held"].bind_function(mouse_left_held);
		globalgetters["mouse_left_pressed"].bind_function(mouse_left_pressed);
		globalgetters["mouse_right_held"].bind_function(mouse_right_held);
		globalgetters["mouse_right_pressed"].bind_function(mouse_right_pressed);

		globals["key_pressed"].bind_function(key_pressed);
		globals["key_held"].bind_function(key_held);

		globals["key_held"].bind_function(key_held);

		globals["screen_coords"].bind_function(screen_coords);
		globals["world_coords"].bind_function(world_coords);

		LuaValue keys = globals["keys"].ensure_table();

		keys["ENTER"] = (int)SDLK_RETURN;
		keys["ESCAPE"] = (int)SDLK_ESCAPE;
		keys["SPACE"] = (int)SDLK_SPACE;
		keys["DELETE"] = (int)SDLK_DELETE;
		keys["UP"] = (int)SDLK_UP;
		keys["DOWN"] = (int)SDLK_DOWN;
		keys["RIGHT"] = (int)SDLK_RIGHT;
		keys["LEFT"] = (int)SDLK_LEFT;
		keys["INSERT"] = (int)SDLK_INSERT;
		keys["HOME"] = (int)SDLK_HOME;
		keys["END"] = (int)SDLK_END;
		keys["PAGEUP"] = (int)SDLK_PAGEUP;
		keys["PAGEDOWN"] = (int)SDLK_PAGEDOWN;
		keys["F1"] = (int)SDLK_F1;
		keys["F2"] = (int)SDLK_F2;
		keys["F3"] = (int)SDLK_F3;
		keys["F4"] = (int)SDLK_F4;
		keys["F5"] = (int)SDLK_F5;
		keys["F6"] = (int)SDLK_F6;
		keys["F7"] = (int)SDLK_F7;
		keys["F8"] = (int)SDLK_F8;
		keys["F9"] = (int)SDLK_F9;
		keys["F10"] = (int)SDLK_F10;
		keys["F11"] = (int)SDLK_F11;
		keys["F12"] = (int)SDLK_F12;
		keys["F13"] = (int)SDLK_F13;
		keys["F14"] = (int)SDLK_F14;
		keys["F15"] = (int)SDLK_F15;
	}
}
