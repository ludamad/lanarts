/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <lua.hpp>

#include <lcommon/Timer.h>
#include <lcommon/math_util.h>

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

static int lua_draw_rectangle_outline(lua_State* L) {
	using namespace ldraw;
	using namespace luawrap;
	draw_rectangle_outline(get<Colour>(L, 1), get<BBox>(L, 2), get_defaulted(L, 3, 1.0f));
	return 0;
}

static void draw_loop(LuaValue draw_func) {
	exit(1); // Not supported in headless mode!
}

namespace ldraw {
void lua_register_ldraw(lua_State* L, const LuaValue& module, bool register_lcommon) {
	using namespace ldraw;

	if (register_lcommon) {
		lua_register_lcommon(L);
	}

	module["draw_rectangle"].bind_function(draw_rectangle);
	module["draw_circle"].bind_function(draw_circle);
	module["draw_circle_outline"].bind_function(draw_circle_outline);
	module["draw_rectangle_outline"].bind_function(lua_draw_rectangle_outline);
	module["draw_line"].bind_function(draw_line);
	module["initialize"].bind_function(display_initialize);
	module["draw_loop"].bind_function(draw_loop);
	module["set_window_region"].bind_function(display_set_window_region);
	module["set_world_region"].bind_function(display_set_world_region);
	module["reset_blend_func"].bind_function(reset_blend_func);

	lua_register_font(L, module);
	lua_register_image(L, module);
	lua_register_drawoptions(L, module);
	lua_register_colour(L, module);
	lua_register_drawables(L, module);
}

}
