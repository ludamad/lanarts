/*
 * lua_core_Keyboard.cpp:
 *  Bindings for keyboard state queries
 */

#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"

#include "lua_api.h"

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

namespace lua_api {
	void register_lua_core_Mouse(lua_State* L) {
		// Mouse API
		LuaModule mouse = lua_api::register_lua_submodule_as_luamodule(L, "core.Mouse");
		mouse.getters["mouse_xy"].bind_function(mouse_xy);
		mouse.getters["mouse_left_held"].bind_function(mouse_left_held);
		mouse.getters["mouse_left_pressed"].bind_function(mouse_left_pressed);
		mouse.getters["mouse_right_held"].bind_function(mouse_right_held);
		mouse.getters["mouse_right_pressed"].bind_function(mouse_right_pressed);
	}
}
