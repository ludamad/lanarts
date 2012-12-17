/*
 * lua_display.cpp:
 *  Display-related routines and functions
 */


#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <SDL.h>

#include "gamestate/GameState.h"
#include "gamestate/GameLevelState.h"
#include "gamestate/GameSettings.h"

#include "lua_newapi.h"

namespace lua_api {

	/* Functions for visual results in the lanarts world, eg drawing text */
	void register_display_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		GameState* gs = lua_api::gamestate(L);

		LuaValue fonts = globals["fonts"].ensure_table();

		fonts["small"] = gs->font();
		fonts["large"] = gs->menu_font();
	}
}
