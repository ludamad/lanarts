/*
 * lua_newapi.cpp:
 *  New API, rename to lua_api once completed
 */

#include <luawrap/luawrap.h>

#include "lua_newapi.h"

namespace lua_api {

	static void register_gamestate(GameState* gs, lua_State* L) {
		lua_pushlightuserdata(L, gs);
		luawrap::registry(L)["__gamestate"].pop();
	}

	// Grab the GameState back-pointer
	// Used to implement lua_api functions that affect GameState
	GameState* gamestate(lua_State* L) {
		luawrap::registry(L)["__gamestate"].push();
		GameState* gs = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return gs;
	}

	// Register the lanarts API functions and types
	void register_api(GameState* gs, lua_State* L) {
		register_gamestate(gs, L);
	}
}
