/*
 * lua_colour.h:
 *  Bindings for Colour
 */

#ifndef LUA_COLOUR_H_
#define LUA_COLOUR_H_

#include <SLB/Table.hpp>
#include <common/lua/slb_valuetype.h>

#include "../Colour.h"

SLB_WRAP_VALUE_TYPE(Colour, (lua_push_as_numarray<Colour, int>),
		(lua_get_from_numarray<Colour, int, 255>),
		(lua_check_as_numarray<Colour, int, 1>));

void lua_register_colour_constants(lua_State *L, SLB::Table* table);

#endif /* LUA_COLOUR_H_ */
