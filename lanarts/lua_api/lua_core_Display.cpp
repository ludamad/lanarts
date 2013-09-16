/*
 * lua_display.cpp:
 *  Display-related routines and functions
 */


#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/macros.h>

#include <lcommon/lua_utils.h>
#include <lcommon/math_util.h>
#include <lcommon/strformat.h>

#include <ldraw/display.h>
#include <ldraw/Image.h>
#include <ldraw/lua_ldraw.h>

#include <SDL.h>

#include "draw/parse_drawable.h"

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"
#include "gamestate/GameSettings.h"

#include "lua_api.h"

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
		luawrap::error(format("Invalid pattern '%s'!\n", pattern));
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

namespace lua_api {

	static void register_display_submodule(lua_State* L) {
		LuaSpecialValue globals = luawrap::globals(L);
		LuaModule display = register_lua_submodule_as_luamodule(L, "core.Display");
		LuaValue vals = display.values;
		ldraw::lua_register_ldraw(L, vals);

		vals["screen_coords"].bind_function(screen_coords);
		vals["world_coords"].bind_function(world_coords);
		display.getters["display_size"].bind_function(ldraw::display_size);
		display.getters["display_xy"].bind_function(display_xy);

		vals["images_load"].bind_function(images_load);
		vals["initialize"].bind_function(ldraw::display_initialize);
		vals["draw_start"].bind_function(ldraw::display_draw_start);
		vals["draw_finish"].bind_function(ldraw::display_draw_finish);
		vals["object_within_view"].bind_function(object_within_view);
		vals["to_screen_xy"].bind_function(to_screen_xy);
	}

	/* Functions for visual results in the lanarts world, eg drawing text */
	void register_lua_core_Display(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		GameState* gs = lua_api::gamestate(L);

		LuaValue fonts = luawrap::ensure_table(globals["Fonts"]);
		LuaValue fonts_meta = luameta_new(L, "Fonts");
		LuaValue fonts_getters = luameta_getters(fonts_meta);
		fonts.set_metatable(fonts_meta);

		LUAWRAP_SET_TYPE(LuaValue);
		LUAWRAP_GETTER(fonts_getters, small, lua_api::gamestate(L)->font());
		LUAWRAP_GETTER(fonts_getters, large, lua_api::gamestate(L)->menu_font());

		register_display_submodule(L);
	}

	void register_event_log_api(lua_State* L) {
		LuaValue event_log = register_lua_submodule(L, "core.ui.EventLog");
		event_log["add"].bind_function(event_log_add);
	}
}
