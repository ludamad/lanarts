/*
 * lua_numarray.h:
 *  Functions for going to and from numeric arrays
 */

#ifndef LUA_NUMARRAY_H_
#define LUA_NUMARRAY_H_

#include <lua/lua.h>

template <typename Num>
inline void lua_push_numarray(lua_State* L, const Num* nums, int n) {
	lua_newtable(L);
	int idx = lua_gettop(L);
	for (int i = 0; i < n; i++) {
		lua_pushnumber(L, nums[i]);
		lua_rawseti(L, idx, i + 1);
	}
}

template <typename Num>
inline void lua_to_numarray(lua_State* L, int idx, Num* nums, int n) {
	for (int i = 0; i < n; i++) {
		lua_rawgeti(L, idx, i + 1);
		if (!lua_isnil(L, -1)) {
			nums[i] = lua_tonumber(L, -1);
		}
		lua_pop(L, 1);
	}
}



#endif /* LUA_NUMARRAY_H_ */
