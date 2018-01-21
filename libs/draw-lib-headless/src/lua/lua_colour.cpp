/*
 * lua_colour.cpp:
 *  Bindings for Colour
 */

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include <lcommon/lua_numeric_tuple_helper.h>

#include "colour_constants.h"

#include "lua_colour.h"

static Colour color_alpha(const Colour& c, float alpha) {
	return c.mult_alpha(alpha);
}

void ldraw::lua_register_colour(lua_State *L,
		const LuaValue& module) {

	typedef LuaNumericTupleFunctions<Colour, int, 1 /*allowed missing*/, 255 /*default*/> ImplClass;
	luawrap::install_type<Colour, ImplClass::push, ImplClass::get,
			ImplClass::check>();

	module["color_alpha"].bind_function(color_alpha);

	LuaSpecialValue globals = luawrap::globals(L);

	globals["COL_GOLD"] = COL_GOLD;
	globals["COL_YELLOW"] = COL_YELLOW;
	globals["COL_MUTED_YELLOW"] = COL_MUTED_YELLOW;
	globals["COL_PALE_YELLOW"] = COL_PALE_YELLOW;

	globals["COL_LIGHT_RED"] = COL_LIGHT_RED;
	globals["COL_PALE_RED"] = COL_PALE_RED;
	globals["COL_RED"] = COL_RED;

	globals["COL_MUTED_GREEN"] = COL_MUTED_GREEN;
	globals["COL_PALE_GREEN"] = COL_PALE_GREEN;
	globals["COL_GREEN"] = COL_GREEN;

	globals["COL_LIGHT_BLUE"] = COL_LIGHT_BLUE;
	globals["COL_BLUE"] = COL_BLUE;
	globals["COL_BABY_BLUE"] = COL_BABY_BLUE;
	globals["COL_PALE_BLUE"] = COL_PALE_BLUE;

	globals["COL_MEDIUM_PURPLE"] = COL_MEDIUM_PURPLE;

	globals["COL_BLACK"] = COL_BLACK;
	globals["COL_DARKER_GRAY"] = COL_DARKER_GRAY;
	globals["COL_DARK_GRAY"] = COL_DARK_GRAY;
	globals["COL_GRAY"] = COL_GRAY;
	globals["COL_MID_GRAY"] = COL_MID_GRAY;
	globals["COL_LIGHT_GRAY"] = COL_LIGHT_GRAY;
	globals["COL_WHITE"] = COL_WHITE;
	globals["COL_INVISIBLE"] = COL_INVISIBLE;

}
