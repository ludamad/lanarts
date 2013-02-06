/*
 * lua_drawoptions.cpp:
 *  Bindings for drawing options <-> lua table
 */

#include <lua.hpp>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/types.h>

#include <lcommon/lua_numeric_tuple_helper.h>

#include "lua_colour.h"
#include "lua_drawoptions.h"

void lua_push_drawoptions(lua_State *L, const ldraw::DrawOptions & options) {
	lua_newtable(L);
	LuaStackValue table(L, -1);

	table["origin"] = options.draw_origin;

	table["color"] = options.draw_colour.clamp();
	table["region"] = options.draw_region;
	table["scale"] = options.draw_scale;
	table["angle"] = options.draw_angle;
	table["frame"] = options.draw_frame;
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

	LuaStackValue table(L, idx);

	ldraw::DrawOptions options;

	table["origin"].push();
	if (!lua_isnil(L, -1)) {
		options.draw_origin = luawrap::pop<ldraw::DrawOrigin>(L);
	}

	table["color"].optionalget(options.draw_colour);
	table["region"].optionalget(options.draw_region);
	table["scale"].optionalget(options.draw_scale);
	table["angle"].optionalget(options.draw_angle);
	table["frame"].optionalget(options.draw_frame);

	return options;
}

bool lua_check_drawoptions(lua_State *L, int idx) {
	return lua_istable(L, idx);
}

static int shift_origin(lua_State* L) {
	using namespace ldraw;
	bool bboxcall = lua_gettop(L) <= 2;
	if (bboxcall) {
		BBoxF bbox = luawrap::get<BBoxF>(L, 1);
		DrawOrigin origin = luawrap::get<DrawOrigin>(L, 2);
		luawrap::push(L, adjusted_for_origin(bbox, origin));
	} else {
		Posf pos = luawrap::get<Posf>(L, 1);
		SizeF size = luawrap::get<SizeF>(L, 2);
		DrawOrigin origin = luawrap::get<DrawOrigin>(L, 3);
		luawrap::push(L, adjusted_for_origin(pos, size, origin));
	}
	return 1;
}


template<typename T, typename V>
static void install_numeric_tuple() {
	typedef LuaNumericTupleFunctions<T, V> ImplClass;
	luawrap::install_type<T, ImplClass::push, ImplClass::get, ImplClass::check>();
}


void ldraw::lua_register_drawoptions(lua_State *L,
		const LuaValue& module) {
	using namespace ldraw;
	install_numeric_tuple<DrawOrigin, float>();
	luawrap::install_type<DrawOptions, lua_push_drawoptions,
			lua_get_drawoptions, lua_check_drawoptions>();

#define BIND_ORIGIN_CONST(origin) \
	module[#origin] = origin

	BIND_ORIGIN_CONST(LEFT_TOP);
	BIND_ORIGIN_CONST(LEFT_CENTER);
	BIND_ORIGIN_CONST(LEFT_BOTTOM);
	BIND_ORIGIN_CONST(CENTER_TOP);
	BIND_ORIGIN_CONST(CENTER);
	BIND_ORIGIN_CONST(CENTER_BOTTOM);
	BIND_ORIGIN_CONST(RIGHT_TOP);
	BIND_ORIGIN_CONST(RIGHT_CENTER);
	BIND_ORIGIN_CONST(RIGHT_BOTTOM);

	module["shift_origin"].bind_function(shift_origin);

}

