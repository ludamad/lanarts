/*
 * lua_core_FieldOfView.cpp:
 *  Field-of-view submodule
 */

#include <SDL.h>

#include <ldungeon_gen/Map.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_newapi.h"

#include "lua_api/lua_newapi.h"
#include "fov/fov.h"

#include <lcommon/math_util.h>

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"

typedef smartptr<fov> FovPtr;
static int fov_update(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	FovPtr fov = luawrap::get<FovPtr>(L, 1);
	level_id gmap = luawrap::get<int>(L, 2);
	Pos xy = luawrap::get<Pos>(L, 3);
	bool clear = lua_gettop(L) >= 5 ? lua_toboolean(L, 5) : true;

	GameMapState* previous_level = gs->get_level();
	gs->set_level(gs->game_world().get_level(gmap));
	fov->calculate(gs, xy.x / TILE_SIZE, xy.y / TILE_SIZE);
	gs->set_level(previous_level);

	return 0;
}

static bool fov_point_visible(FovPtr fov, Pos xy) {
	return fov->within_fov(xy.x/TILE_SIZE, xy.y/TILE_SIZE);
}

static bool fov_rectangle_visible(FovPtr fov, BBox rect) {
	int w = rect.width(), h = rect.height();
	int mingrid_x = rect.x1 / TILE_SIZE, mingrid_y = rect.y1 / TILE_SIZE;
	int maxgrid_x = rect.x2 / TILE_SIZE, maxgrid_y = rect.y2 / TILE_SIZE;
	return fov->within_fov(BBox(mingrid_x, mingrid_y, maxgrid_x, maxgrid_y));
}

static bool fov_circle_visible(FovPtr fov, Pos xy, int radius) {
	return fov_rectangle_visible(fov,
		BBox(xy - Pos(radius, radius), Size(radius * 2, radius * 2))
	);
}

LuaValue lua_fovmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "FieldOfView");
	LuaValue methods = luameta_constants(meta);
	methods["update"].bind_function(fov_update);
	methods["point_visible"].bind_function(fov_point_visible);
	methods["rectangle_visible"].bind_function(fov_rectangle_visible);
	methods["circle_visible"].bind_function(fov_circle_visible);

	luameta_gc<FovPtr>(meta);

	return meta;
}

static FovPtr field_of_view_create(int width) {
	return FovPtr(new fov(width));
}

namespace lua_api {
	void register_lua_core_FieldOfView(lua_State* L) {
		LuaValue fov = register_lua_submodule(L, "core.FieldOfView");
		luawrap::install_userdata_type<FovPtr, &lua_fovmetatable>();
		fov["create"].bind_function(field_of_view_create);
	}
}
