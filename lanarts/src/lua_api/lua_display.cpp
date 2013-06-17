/*
 * lua_display.cpp:
 *  Display-related routines and functions
 */


#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <ldraw/display.h>
#include <ldraw/Image.h>

#include <SDL.h>

#include "draw/parse_drawable.h"

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"
#include "gamestate/GameSettings.h"

#include "lua_newapi.h"

static int event_log_add(lua_State* L) {
	int nargs = lua_gettop(L);
	GameState* gs = lua_api::gamestate(L);
	Colour col = nargs >= 2 ? LuaStackValue(L, 2).as<Colour>() : Colour();
	gs->game_chat().add_message(LuaStackValue(L, 1).to_str(), col);
	return 0;
}

static std::vector<ldraw::Image> images_load(const char* pattern) {
	std::vector<ldraw::Image> ret;
	if (!filepattern_to_image_list(ret,pattern)) {
		luawrap::error("Invalid pattern!\n");
	}
	return ret;
}

namespace lua_api {

	static void register_display_table(lua_State* L) {
		LuaSpecialValue globals = luawrap::globals(L);
		LuaValue display = luawrap::ensure_table(globals["Display"]);

		globals["images_load"].bind_function(images_load);
		display["initialize"].bind_function(ldraw::display_initialize);
		display["draw_start"].bind_function(ldraw::display_draw_start);
		display["draw_finish"].bind_function(ldraw::display_draw_finish);

		LuaValue meta = luameta_new(L, "<DisplayTable>");
		LuaValue getters = luameta_getters(meta);

		getters["display_size"].bind_function(ldraw::display_size);

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

	void register_event_log_api(lua_State* L) {
		LuaSpecialValue globals = luawrap::globals(L);
		LuaValue event_log = luawrap::ensure_table(globals["EventLog"]);
		event_log["add"].bind_function(event_log_add);
	}
}
