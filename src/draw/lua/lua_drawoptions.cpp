/*
 * lua_drawoptions.cpp:
 *  Bindings for drawing options <-> lua table
 */

#include <common/lua/slb_valuetype.h>
#include <lua/lua.h>

#include "lua_drawoptions.h"

void lua_push_drawoptions(lua_State *L, const ldraw::DrawOptions & options) {
	lua_newtable(L);
	int tblidx = lua_gettop(L);

	SLB::table_set(L, tblidx, "origin", int(options.draw_origin));
	SLB::table_set(L, tblidx, "colour", options.draw_colour);
	SLB::table_set(L, tblidx, "region", options.draw_region);
}

ldraw::DrawOptions lua_get_drawoptions(lua_State *L, int idx) {
	ldraw::DrawOptions options;
	SLB::table_get<int>(L, idx, "origin", options.draw_origin);
	SLB::table_get(L, idx, "colour", options.draw_colour);
	SLB::table_get(L, idx, "region", options.draw_region);
	return options;
}

bool lua_check_drawoptions(lua_State *L, int idx) {
	return lua_istable(L, idx);
}

