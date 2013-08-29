/*
 * lua_display.cpp:
 *  Display-related routines and functions
 */


#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/macros.h>

#include <lcommon/lua_utils.h>
#include <lcommon/math_util.h>

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

static int display_xy(lua_State* L) {
	GameView& view = lua_api::gamestate(L)->view();
	luawrap::push<Pos>(L, Pos(view.x, view.y));
	return 1;
}

static int to_screen_xy(lua_State* L) {
	GameView& view = lua_api::gamestate(L)->view();
	luawrap::push<Pos>(L, luawrap::get<Pos>(L, 1) - Pos(view.x, view.y));
	return 1;
}

static int object_within_view(lua_State* L) {
	GameView& view = lua_api::gamestate(L)->view();
	GameInst* inst = luawrap::get<GameInst*>(L, 1);
	lua_pushboolean(L, circle_rectangle_test(inst->pos(), inst->target_radius, view.region_covered()));
	return 1;
}

namespace lua_api {

	static void register_display_table(lua_State* L) {
		LuaSpecialValue globals = luawrap::globals(L);
		LuaValue display = register_lua_submodule(L, "core.Display");

		globals["images_load"].bind_function(images_load);
		display["initialize"].bind_function(ldraw::display_initialize);
		display["draw_start"].bind_function(ldraw::display_draw_start);
		display["draw_finish"].bind_function(ldraw::display_draw_finish);
		display["object_within_view"].bind_function(object_within_view);
		display["to_screen_xy"].bind_function(to_screen_xy);

		LuaValue meta = luameta_new(L, "<DisplayTable>");
		LuaValue getters = luameta_getters(meta);

		getters["display_size"].bind_function(ldraw::display_size);
		getters["display_xy"].bind_function(display_xy);

		display.set_metatable(meta);
	}

	/* Functions for visual results in the lanarts world, eg drawing text */
	void register_display_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		GameState* gs = lua_api::gamestate(L);

		LuaValue fonts = luawrap::ensure_table(globals["Fonts"]);
		LuaValue fonts_meta = luameta_new(L, "Fonts");
		LuaValue fonts_getters = luameta_getters(fonts_meta);
		fonts.set_metatable(fonts_meta);

		LUAWRAP_SET_TYPE(LuaValue);
		LUAWRAP_GETTER(fonts_getters, small, lua_api::gamestate(L)->font());
		LUAWRAP_GETTER(fonts_getters, large, lua_api::gamestate(L)->menu_font());

		register_display_table(L);
	}

	void register_event_log_api(lua_State* L) {
		LuaValue event_log = register_lua_submodule(L, "core.ui.EventLog");
		event_log["add"].bind_function(event_log_add);
	}
}
