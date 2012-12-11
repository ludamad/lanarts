/*
 * lua_colour.cpp:
 *  Bindings for Colour
 */

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/types.h>

#include <common/lua/lua_numeric_tuple_helper.h>

#include "../colour_constants.h"

#include "lua_colour.h"

void ldraw::lua_register_colour(lua_State *L,
		const LuaValue& module) {

	typedef LuaNumericTupleFunctions<Colour, int, 1 /*allowed missing*/, 255 /*default*/> ImplClass;
	luawrap::install_type<Colour, ImplClass::push, ImplClass::get,
			ImplClass::check>();

#define BIND_COLOUR(col) \
	module.get(L, #col) = col

	BIND_COLOUR(COL_GOLD);
	BIND_COLOUR(COL_YELLOW);
	BIND_COLOUR(COL_MUTED_YELLOW);
	BIND_COLOUR(COL_PALE_YELLOW);

	BIND_COLOUR(COL_LIGHT_RED);
	BIND_COLOUR(COL_PALE_RED);
	BIND_COLOUR(COL_RED);

	BIND_COLOUR(COL_MUTED_GREEN);
	BIND_COLOUR(COL_PALE_GREEN);
	BIND_COLOUR(COL_GREEN);

	BIND_COLOUR(COL_LIGHT_BLUE);
	BIND_COLOUR(COL_BLUE);
	BIND_COLOUR(COL_BABY_BLUE);
	BIND_COLOUR(COL_PALE_BLUE);

	BIND_COLOUR(COL_MEDIUM_PURPLE);

	BIND_COLOUR(COL_BLACK);
	BIND_COLOUR(COL_DARKER_GRAY);
	BIND_COLOUR(COL_DARK_GRAY);
	BIND_COLOUR(COL_GRAY);
	BIND_COLOUR(COL_MID_GRAY);
	BIND_COLOUR(COL_LIGHT_GRAY);
	BIND_COLOUR(COL_WHITE);

}
