/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <SLB/FuncCall.hpp>

#include <common/lua/slb_valuetype.h>
#include <common/lua/LuaValue.h>

#include "../colour_constants.h"
#include "../draw.h"
#include "../Image.h"

#include "lua_ldraw.h"

static ldraw::Image load_image(const std::string& filename) {
	return ldraw::Image(filename);
}

static ldraw::Font load_font(const std::string& filename, int height) {
	return ldraw::Font(filename, height);
}

void lua_register_ldraw(lua_State* L, LuaValue& module) {
	using namespace ldraw;
#define BIND_FUNC(f)\
	SLB::FuncCall::create(f)->push(L); \
	module.get(L, #f).pop()

	BIND_FUNC(draw_rectangle);
	BIND_FUNC(draw_circle);
	BIND_FUNC(draw_circle_outline);
	BIND_FUNC(draw_rectangle_outline);
	BIND_FUNC(load_image);
	BIND_FUNC(load_font);

	lua_register_image(L);
	lua_register_font(L);
	lua_register_draworigin_constants(L, module);
	lua_register_colour_constants(L, module);
}
