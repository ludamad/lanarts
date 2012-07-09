/*
 * lua_impl_util.h:
 *  Provides common routines for handling arguments passed to lua functions
 */

#ifndef LUA_IMPL_UTIL_H_
#define LUA_IMPL_UTIL_H_

#include <lua/lunar.h>
#include "../data/lua_game_data.h"

Colour colour_from_lua(lua_State* L, int idx);

#endif /* LUA_IMPL_UTIL_H_ */
