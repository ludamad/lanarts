/*
 * lua_lcommon.h:
 *  Lua bindings for lcommon library
 */

#ifndef LUA_LCOMMON_H_
#define LUA_LCOMMON_H_

#include <SLB/Table.hpp>

#include "lua_range.h"
#include "lua_timer.h"
#include "lua_vector.h"
#include "lua_geometry.h"

void lua_register_lcommon(lua_State* L, SLB::Table* module);

#endif /* LUA_LCOMMON_H_ */
