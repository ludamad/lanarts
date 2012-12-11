/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <luawrap/LuaValue.h>
#include <luawrap/functions.h>

#include <common/lua/lua_geometry.h>
#include <common/lua/lua_lcommon.h>

#include "../colour_constants.h"
#include "../draw.h"
#include "../Image.h"

#include "lua_image.h"
#include "lua_font.h"
#include "lua_colour.h"
#include "lua_drawable.h"
#include "lua_drawoptions.h"

#include "lua_ldraw.h"

namespace ldraw {

void lua_register_ldraw(lua_State* L, const LuaValue& module) {
	using namespace ldraw;
#define BIND_FUNC(f)\
	module.get(L, #f) = luawrap::function(L, f)

	lua_register_lcommon(L, module);

	BIND_FUNC(draw_rectangle);
	BIND_FUNC(draw_circle);
	BIND_FUNC(draw_circle_outline);
	BIND_FUNC(draw_rectangle_outline);

	lua_register_font(L, module);
	lua_register_image(L, module);
	lua_register_drawoptions(L, module);
	lua_register_colour(L, module);
	lua_register_drawables(L, module);
}

}
