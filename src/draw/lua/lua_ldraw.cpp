/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <SLB/FuncCall.hpp>

#include <common/lua/luacpp_wrap.h>
#include <common/lua/LuaValue.h>

#include "../colour_constants.h"
#include "../draw.h"
#include "../Image.h"

#include "lua_image.h"
#include "lua_font.h"

#include "lua_ldraw.h"

namespace ldraw {

void lua_register_ldraw(lua_State* L, const LuaValue& module) {
	using namespace ldraw;
#define BIND_FUNC(f)\
	SLB::FuncCall::create(f)->push(L); \
	module.get(L, #f).pop()

	BIND_FUNC(draw_rectangle);
	BIND_FUNC(draw_circle);
	BIND_FUNC(draw_circle_outline);
	BIND_FUNC(draw_rectangle_outline);

	lua_register_font(L, module);
	lua_register_image(L, module);
	lua_register_draworigin_constants(L, module);
	lua_register_colour_constants(L, module);
	lua_register_drawables(L, module);
}

}
