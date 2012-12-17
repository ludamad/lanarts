/*
 * lua_display_funcs.cpp:
 *  Functions that generally affect the visual state of the game
 */

#include <lua.hpp>

#include "data/lua_game_data.h"

#include "gamestate/GameState.h"

#include "lua_api.h"

// Creates object, adding to game world, returns said object
// Must ensure that it is being called during the draw phase
// TODO: Some sort of step/draw check that throws a panic ?
// Take arguments: sprite, x, y
static int draw_sprite(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gl_draw_sprite(gs->view(), sprite_from_lua(L, 1), lua_tointeger(L, 2),
			lua_tointeger(L, 3));
	return 0;
}

static int show_message(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int nargs = lua_gettop(L);
	Colour col = nargs >= 2 ? lua_tocolour(L, 2) : Colour();
	gs->game_chat().add_message(lua_tocppstring(L, 1), col);
	return 0;
}

void lua_display_func_bindings(lua_State* L) {
	//Use C function name as lua function name:
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);
	LUA_FUNC_REGISTER(draw_sprite);
	LUA_FUNC_REGISTER(show_message);
}
