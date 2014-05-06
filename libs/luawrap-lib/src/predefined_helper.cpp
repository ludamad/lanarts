/*
 * predefined_helper.cpp:
 *  Predefined wrappers for get/check/push. These types cannot be customized (other
 *  than by manually editing this file).
 *  Use LUAWRAP_NO_WRAP_VECTOR to not wrap vector.
 */

#include <cstdio>

#include "predefined_helper.h"

namespace luawrap {
	namespace _private {

		void TypeImpl<LuaValue>::push(lua_State* L, const LuaValue& val) {
			val.push();
		}

		LuaValue TypeImpl<LuaValue>::get(lua_State* L, int idx) {
			return LuaValue(L, idx);
		}

		bool TypeImpl<LuaValue>::check(lua_State* L, int idx) {
			return true;
		}

		void TypeImpl<LuaStackValue>::push(lua_State* L, const LuaStackValue& val) {
			val.push();
		}

		LuaStackValue TypeImpl<LuaStackValue>::get(lua_State* L, int idx) {
			return LuaStackValue(L, idx);
		}

		bool TypeImpl<LuaStackValue>::check(lua_State* L, int idx) {
			return true;
		}

		void TypeImpl<LuaField>::push(lua_State* L, const LuaField& val) {
			val.push();
		}

		LuaField TypeImpl<LuaField>::get(lua_State* L, int idx) {
			return LuaStackValue(L, idx);
		}

		bool TypeImpl<LuaField>::check(lua_State* L, int idx) {
			return true;
		}

		void TypeImpl<lua_CFunction>::push(lua_State* L, lua_CFunction val) {
			lua_pushcfunction(L, val);
		}

		lua_CFunction TypeImpl<lua_CFunction>::get(lua_State* L, int idx) {
			return lua_tocfunction(L, idx);
		}

		bool TypeImpl<lua_CFunction>::check(lua_State* L, int idx) {
			return lua_iscfunction(L, idx);
		}

		void TypeImpl<bool>::push(lua_State* L, bool val) {
			lua_pushboolean(L, val);
		}

		bool TypeImpl<bool>::get(lua_State* L, int idx) {
			return lua_toboolean(L, idx);
		}

		bool TypeImpl<bool>::check(lua_State* L, int idx) {
			return lua_isboolean(L, idx);
		}

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

		void TypeImpl<std::string>::push(lua_State* L, const char* val) {
			lua_pushstring(L, val);
		}
		const char* TypeImpl<std::string>::get(lua_State* L, int idx) {
			return lua_tostring(L, idx);
		}

		bool TypeImpl<std::string>::check(lua_State *L, int idx) {
			return lua_isstring(L, idx);
		}

	}
}
