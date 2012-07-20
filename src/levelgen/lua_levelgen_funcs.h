/*
 * lua_levelgen_funcs.h:
 *  Lua level gen callbacks
 */

#ifndef LUA_LEVELGEN_FUNCS_H_
#define LUA_LEVELGEN_FUNCS_H_

#include "GeneratedLevel.h"

struct lua_State* L;

void lua_generatedlevel_push(lua_State* L, GeneratedLevel& level);
void lua_generatedlevel_invalidate(lua_State* L, int idx);

void lua_levelgen_func_bindings(lua_State* L);

#endif /* LUA_LEVELGEN_FUNCS_H_ */
