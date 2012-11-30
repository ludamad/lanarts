/*
 * luacpp_wrap_function.h:
 *  Binding for C++ functions to Lua
 */

#ifndef LUACPP_WRAP_FUNCTION_H_
#define LUACPP_WRAP_FUNCTION_H_

#include <SLB/FuncCall.hpp>

/**
 * XXX:
 * NOTE: As written, this function leaks memory!
 * Use only on startup until a fix is found.
 */
template<typename T>
inline void lua_pushcppfunction(lua_State* L, const T& value) {
	SLB::FuncCall::create(value)->push(L);
}

/**
 * XXX:
 * NOTE: As written, this function leaks memory!
 * Use only on startup until a fix is found.
 */
template<typename T>
inline LuaValue luavalue_from_function(lua_State* L, const T& value) {
	SLB::FuncCall::create(value)->push(L);
	LuaValue val(L, -1);
	lua_pop(L, 1);
	return val;
}

#endif /* LUACPP_WRAP_FUNCTION_H_ */
