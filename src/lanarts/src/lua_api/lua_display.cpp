/*
 * lua_display.cpp:
 *  Display-related routines and functions
 */


#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <ldraw/display.h>

#include <SDL.h>

#include "gamestate/GameState.h"
#include "gamestate/GameRoomState.h"
#include "gamestate/GameSettings.h"

#include "lua_newapi.h"

namespace lua_api {

	static void register_display_table(lua_State* L) {
		LuaValue display = luawrap::ensure_table(luawrap::globals(L)["Display"]);

		display["initialize"].bind_function(ldraw::display_initialize);
		display["draw_start"].bind_function(ldraw::display_draw_start);
		display["draw_finish"].bind_function(ldraw::display_draw_finish);

		LuaValue meta = luameta_new(L, "<DisplayTable>");
		LuaValue getters = luameta_getters(meta);

		getters["window_size"].bind_function(ldraw::display_size);

		display.push();
		meta.push();
		lua_setmetatable(L, -2);
		lua_pop(L, 1);
	}


	/* Functions for visual results in the lanarts world, eg drawing text */
	void register_display_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		GameState* gs = lua_api::gamestate(L);

		LuaValue fonts = luawrap::ensure_table(globals["Fonts"]);

		fonts["small"] = gs->font();
		fonts["large"] = gs->menu_font();

		register_display_table(L);
	}
}
