/*
 * lua_vector.h:
 *  Handles conversion from vector <-> lua table
 */

#ifndef LUA_VECTOR_H_
#define LUA_VECTOR_H_

#include <vector>

#include "luacpp_wrap.h"

template<typename T>
inline void lua_push_vector(lua_State* L, const std::vector<T>& value) {
	lua_newtable(L);
	int idx = lua_gettop(L);
	for (int i = 0; i < value.size(); i++) {
		SLB::push<T>(L, value[i]);
		lua_rawseti(L, idx, i + 1);
	}
}

template<typename T>
inline std::vector<T> lua_get_vector(lua_State* L, int idx) {
	std::vector<T> value;

	int size = lua_objlen(L, idx);
	for (int i = 0; i < size; i++) {
		lua_rawgeti(L, idx, i + 1);
		value.push_back(SLB::get<T>(L, -1));

		lua_pop(L, 1);
	}
	return value;
}
template<typename T>
bool lua_check_vector(lua_State *L, int idx) {
	int size = lua_objlen(L, idx);
	bool valid = true;

	for (int i = 0; i < size && valid; i++) {
		lua_rawgeti(L, idx, i + 1);
		if (!SLB::check<T>(L, -1)) {
			valid = false;
		}

		lua_pop(L, 1);
	}
	return valid;
}

LUACPP_GENERAL_WRAP( typename T, const std::vector<T>&, std::vector<T>,
		std::vector<T>, lua_push_vector<T>, lua_get_vector<T>,
		lua_check_vector<T>);

LUACPP_GENERAL_WRAP( typename T, const std::vector<T>&, const std::vector<T>&,
		std::vector<T>, lua_push_vector<T>, lua_get_vector<T>,
		lua_check_vector<T>);

#endif /* LUA_VECTOR_H_ */
