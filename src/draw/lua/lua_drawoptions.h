/*
 * lua_drawoptions.h:
 *  Bindings for drawing options <-> lua table
 */

#ifndef LUA_DRAWOPTIONS_H_
#define LUA_DRAWOPTIONS_H_

#include <SLB/Table.hpp>

#include <common/lua/slb_valuetype.h>

#include "../DrawOptions.h"

void lua_push_drawoptions(lua_State* L, const ldraw::DrawOptions& options);
ldraw::DrawOptions lua_get_drawoptions(lua_State* L, int idx);
bool lua_check_drawoptions(lua_State* L, int idx);

SLB_WRAP_VALUE_TYPE(ldraw::DrawOptions, lua_push_drawoptions,
		lua_get_drawoptions, lua_check_drawoptions);

// Register draworigin constants
void lua_register_draworigin_constants(lua_State* L, SLB::Table* table);

#endif /* LUA_DRAWOPTIONS_H_ */
