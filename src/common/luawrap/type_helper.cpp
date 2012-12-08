/*
 * predefined_helper.cpp:
 *  Predefined wrappers for get/check/push. These types cannot be customized (other
 *  than by manually editing this file).
 *  Use LUAWRAP_NO_WRAP_VECTOR to not wrap vector.
 */

#include <cstdio>

#include "type_helper.h"

namespace luawrap {
namespace _private {

void TypeImpl<double>::push(lua_State* L, double val) {
	lua_pushnumber(L, val);
}

double TypeImpl<double>::get(lua_State* L, int idx) {
	return lua_tonumber(L, idx);
}

bool TypeImpl<double>::check(lua_State* L, int idx) {
	return lua_isnumber(L, idx);
}

void TypeImpl<long>::push(lua_State* L, double val) {
	lua_pushinteger(L, val);
}

long TypeImpl<long>::get(lua_State* L, int idx) {
	return lua_tointeger(L, idx);
}

bool TypeImpl<long>::check(lua_State* L, int idx) {
	return lua_isnumber(L, idx);
}

void TypeImpl<std::string>::push(lua_State* L, const std::string& val) {
	lua_pushlstring(L, val.c_str(), val.size());
}
const char* TypeImpl<std::string>::get(lua_State* L, int idx) {
	return lua_tostring(L, idx);
}
bool TypeImpl<std::string>::check(lua_State *L, int idx) {
	return lua_isstring(L, idx);
}

}
}
