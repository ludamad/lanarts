/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <luawrap/LuaValue.h>
#include <luawrap/functions.h>

#include <lcommon/lua_geometry.h>
#include <lcommon/lua_lcommon.h>

#include "colour_constants.h"
#include "draw.h"
#include "display.h"
#include "Image.h"

#include "lua_image.h"
#include "lua_font.h"
#include "lua_colour.h"
#include "lua_drawable.h"
#include "lua_drawoptions.h"

#include "lua_ldraw.h"

namespace ldraw {

void lua_register_ldraw(lua_State* L, const LuaValue& module, bool register_lcommon) {
	using namespace ldraw;

	if (register_lcommon) {
		lua_register_lcommon(L, module);
	}

	module["draw_rectangle"].bind_function(draw_rectangle);
	module["draw_circle"].bind_function(draw_circle);
	module["draw_circle_outline"].bind_function(draw_circle_outline);
	module["draw_rectangle_outline"].bind_function(draw_rectangle_outline);
	module["draw_line"].bind_function(draw_line);

	lua_register_font(L, module);
	lua_register_image(L, module);
	lua_register_drawoptions(L, module);
	lua_register_colour(L, module);
	lua_register_drawables(L, module);
}

}
