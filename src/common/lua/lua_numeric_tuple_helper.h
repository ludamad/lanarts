/*
 * lua_numeric_tuple_helper.h:
 *  Helpers for defining numeric vector types
 */

#ifndef LUA_NUMERIC_TUPLE_HELPER_H_
#define LUA_NUMERIC_TUPLE_HELPER_H_

#include <lua.hpp>

template<typename T, typename Num, int allowed_missing = 0, int default_value =
		0>
class LuaNumericTupleFunctions {
public:
	static void push(lua_State* L, const T& value) {
		lua_newtable(L);
		const Num* nums = (const Num*)&value;
		int idx = lua_gettop(L);
		for (int i = 0; i < sizeof(T) / sizeof(Num); i++) {
			lua_pushnumber(L, nums[i]);
			lua_rawseti(L, idx, i + 1);
		}
	}

	static T get(lua_State* L, int idx) {
		T value;
		Num* nums = (Num*)&value;
		for (int i = 0; i < sizeof(T) / sizeof(Num); i++) {
			lua_rawgeti(L, idx, i + 1);
			if (!lua_isnil(L, -1)) {
				nums[i] = lua_tonumber(L, -1);
			} else {
				nums[i] = default_value;
			}
			lua_pop(L, 1);
		}
		return value;
	}
	static bool check(lua_State *L, int idx) {
		const int n = sizeof(T) / sizeof(Num);
		const int mincheckn = sizeof(T) / sizeof(Num) - allowed_missing;
		int olen = lua_objlen(L, idx);

		if (!lua_istable(L, idx) || olen < mincheckn || olen > n) {
			return false;
		}
		bool valid = true;
		for (int i = 0; i < mincheckn && valid; i++) {
			lua_rawgeti(L, idx, i + 1);
			if (!lua_isnumber(L, -1)) {
				valid = false;
			}
			lua_pop(L, 1);
		}
		return valid;
	}
};

#endif /* LUA_NUMERIC_TUPLE_HELPER_H_ */
