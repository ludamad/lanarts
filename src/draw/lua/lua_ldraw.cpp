/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <SLB/FuncCall.hpp>

#include <common/lua/slb_valuetype.h>

#include "../colour_constants.h"
#include "../draw.h"
#include "../Image.h"

#include "lua_ldraw.h"

static ldraw::Image load_image(const std::string& filename) {
	return ldraw::Image(filename);
}

void lua_register_ldraw(lua_State* L, SLB::Table* table) {
	using namespace ldraw;
#define BIND_FUNC(f)\
	table->set(#f, SLB::FuncCall::create(f))

	BIND_FUNC(draw_rectangle);
	BIND_FUNC(draw_circle);
	BIND_FUNC(draw_circle_outline);
	BIND_FUNC(draw_rectangle_outline);
	BIND_FUNC(load_image);

	lua_register_image(L);
	lua_register_draworigin_constants(L, table);
	lua_register_colour_constants(L, table);
}
