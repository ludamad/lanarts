/*
 * lua_drawoptions.cpp:
 *  Bindings for drawing options <-> lua table
 */

#include <lua.hpp>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/types.h>

#include "lua_colour.h"
#include "lua_drawoptions.h"

void lua_push_drawoptions(lua_State *L, const ldraw::DrawOptions & options) {
	lua_newtable(L);
	int tblidx = lua_gettop(L);

	luawrap::setfield(L, tblidx, "origin", int(options.draw_origin));
	luawrap::setfield(L, tblidx, "color", options.draw_colour);
	luawrap::setfield(L, tblidx, "region", options.draw_region);
	luawrap::setfield(L, tblidx, "scale", options.draw_scale);
	luawrap::setfield(L, tblidx, "angle", options.draw_angle);
	luawrap::setfield(L, tblidx, "frame", options.draw_frame);
}

ldraw::DrawOptions lua_get_drawoptions(lua_State *L, int idx) {
	lua_rawgeti(L, idx, 1);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
	} else {
		lua_pop(L, 1);
		Colour col = luawrap::get<Colour>(L, idx);
		return col;
	}
	ldraw::DrawOptions options;
	options.draw_origin = (ldraw::DrawOrigin) luawrap::getfield<int>(L, idx,
			"origin");

	luawrap::getoptfield(L, idx, "color", options.draw_colour);
	luawrap::getoptfield(L, idx, "region", options.draw_region);
	luawrap::getoptfield(L, idx, "scale", options.draw_scale);
	luawrap::getoptfield(L, idx, "angle", options.draw_angle);
	luawrap::getoptfield(L, idx, "frame", options.draw_frame);
	return options;
}

bool lua_check_drawoptions(lua_State *L, int idx) {
	return lua_istable(L, idx);
}

void ldraw::lua_register_drawoptions(lua_State *L,
		const LuaValue& module) {
	using namespace ldraw;
	luawrap::install_type<DrawOptions, lua_push_drawoptions,
			lua_get_drawoptions, lua_check_drawoptions>();

#define BIND_ORIGIN_CONST(origin) \
	module[#origin] = +origin

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

