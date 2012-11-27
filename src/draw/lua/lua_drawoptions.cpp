/*
 * lua_drawoptions.cpp:
 *  Bindings for drawing options <-> lua table
 */
extern "C" {
#include <lua/lua.h>
}

#include <common/lua/luacpp_wrap.h>
#include <common/lua/lua_geometry.h>
#include <common/lua/LuaValue.h>
#include <common/lua/luacpp.h>

#include "lua_colour.h"
#include "lua_drawoptions.h"

LUACPP_TYPE_WRAP_IMPL(ldraw::DrawOptions);

void lua_push_drawoptions(lua_State *L, const ldraw::DrawOptions & options) {
	lua_newtable(L);
	int tblidx = lua_gettop(L);

	luatable_set(L, tblidx, "origin", int(options.draw_origin));
	luatable_set(L, tblidx, "color", options.draw_colour);
	luatable_set(L, tblidx, "region", options.draw_region);
	luatable_set(L, tblidx, "scale", options.draw_scale);
	luatable_set(L, tblidx, "angle", options.draw_angle);
	luatable_set(L, tblidx, "frame", options.draw_frame);
}

ldraw::DrawOptions lua_get_drawoptions(lua_State *L, int idx) {
	lua_rawgeti(L, idx, 1);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
	} else {
		lua_pop(L, 1);
		Colour col = luacpp_get<Colour>(L, idx);
		return col;
	}
	ldraw::DrawOptions options;
	luatable_get<int>(L, idx, "origin", options.draw_origin);
	luatable_get(L, idx, "color", options.draw_colour);
	luatable_get(L, idx, "region", options.draw_region);
	luatable_get(L, idx, "scale", options.draw_scale);
	luatable_get(L, idx, "angle", options.draw_angle);
	luatable_get(L, idx, "frame", options.draw_frame);
	return options;
}

bool lua_check_drawoptions(lua_State *L, int idx) {
	return lua_istable(L, idx);
}

void ldraw::lua_register_draworigin_constants(lua_State *L,
		const LuaValue& module) {
	using namespace ldraw;
#define BIND_ORIGIN_CONST(origin) \
	module.get(L, #origin) = +origin

	BIND_ORIGIN_CONST(LEFT_TOP);
	BIND_ORIGIN_CONST(LEFT_CENTER);
	BIND_ORIGIN_CONST(LEFT_BOTTOM);
	BIND_ORIGIN_CONST(CENTER_TOP);
	BIND_ORIGIN_CONST(CENTER);
	BIND_ORIGIN_CONST(CENTER_BOTTOM);
	BIND_ORIGIN_CONST(RIGHT_TOP);
	BIND_ORIGIN_CONST(RIGHT_CENTER);
	BIND_ORIGIN_CONST(RIGHT_BOTTOM);

}

