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

#define BIND_COLOUR(col) \
	lua_pushstring(L, #col);\
	SLB::push(L, col); \
	table->setCache(L)

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

	lua_register_image(L);
}
