/*
 * luacpp_metatable.cpp:
 *  Helpers for creating a flexible metatable for lua classes holding C++ data
 *  Relies on luacpp_push, luacpp_get, luacpp_check
 */

#include <lua.hpp>

#include <luawrap/luameta.h>
#include "luawrapassert.h"

// Takes obj, key
static int luameta_get_member(lua_State* L) {
	LUAWRAP_ASSERT(lua_gettop(L) == 2);

	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1)); // Push getter

	if (lua_isnil(L, -1)) {
		lua_pushvalue(L, 2); // key
		lua_rawget(L, lua_upvalueindex(2));

		if (!lua_isnil(L, -1)) {
			return 1;
		}

		LuaValue tbl(L, lua_upvalueindex(1));
		tbl.get(L, "__typename").push();
		return luaL_error(L, "Type '%s' does not have a '%s' member!\n",
				lua_tostring(L, -1), lua_tostring(L, 2));
	}

	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_call(L, 2, 1); // Call getter with obj, key

	return 1;
}

// Takes obj, key, val
static int luameta_set_member(lua_State* L) {
	LUAWRAP_ASSERT(lua_gettop(L) == 3);

	lua_pushvalue(L, 2); // Push key
	lua_gettable(L, lua_upvalueindex(1)); // Push setter

	if (lua_isnil(L, -1)) {
		LuaValue tbl(L, lua_upvalueindex(1));
		tbl.get(L, "__typename").push();
		return luaL_error(L,
				"Type '%s' does not have a '%s' member, or it is read-only!\n",
				lua_tostring(L, -1), lua_tostring(L, 2));
	}

	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	lua_call(L, 3, 1); // Call setter with obj, key, val

	return 1;
}

/*
 * Creates a new metatable that has getters, setters, and methods cleanly separated.
 * Getters and setters are checked first, and then a method is looked-up.
 */
LuaValue luameta_new(lua_State* L, const char* classname) {
	int pretop = lua_gettop(L);

	LuaValue methodtable;
	methodtable.table_initialize(L);

	LuaValue metatable;
	metatable.table_initialize(L);

	{ // Set up 'getter' lookup, falls back to 'membertable'
		LuaValue gettertable;
		gettertable.table_initialize(L);

		lua_pushstring(L, classname);
		gettertable.get(L, "__typename").pop();

		gettertable.push(L);
		methodtable.push(L);
		lua_pushcclosure(L, luameta_get_member, 2);
		metatable.get(L, "__index").pop();

		metatable.get(L, "__getters") = gettertable;
	}

	{ // Set up 'setter' function table
		LuaValue settertable;
		settertable.table_initialize(L);

		lua_pushstring(L, classname);
		settertable.get(L, "__typename").pop();

		settertable.push(L);
		lua_pushcclosure(L, luameta_set_member, 1);
		metatable.get(L, "__newindex").pop();

		metatable.get(L, "__setters") = settertable;
	}

//	methodtable.push(L); // Set consttable as its own metatable
//	methodtable.push(L); // So that the __index operations chain
//	lua_setmetatable(L, -2);
//	lua_pop(L, 1);
	/*pop metatable*/

	metatable.get(L, "__methods") = methodtable;

	LUAWRAP_ASSERT(lua_gettop(L) == pretop);

	return metatable;
}

LuaValue luameta_getters(lua_State* L, const LuaValue& metatable) {
	return metatable.get(L, "__getters");
}

LuaValue luameta_setters(lua_State* L, const LuaValue& metatable) {
	return metatable.get(L, "__setters");
}

LuaValue luameta_methods(lua_State* L, const LuaValue& metatable) {
	return metatable.get(L, "__methods");
}

/*
 * Pushes cached metatable if exists, or initializes cache and then pushes.
 */
void luameta_push(lua_State* L, luameta_initializer initfunc) {
	lua_pushlightuserdata(L, (void*)initfunc);
	lua_gettable(L, LUA_REGISTRYINDEX);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		LuaValue metatable = initfunc(L);
		lua_pushlightuserdata(L, (void*)initfunc);
		metatable.push(L);
		lua_settable(L, LUA_REGISTRYINDEX);

		metatable.push(L);
	}
}

/*
 * Pushes a new, uninitialized userdata with the given metatable.
 * Note that the metatable is cached (via luameta_push).
 */
void* luameta_newuserdata(lua_State* L, luameta_initializer initfunc,
		size_t size) {
	void* ret = lua_newuserdata(L, size);
	luameta_push(L, initfunc);
	lua_setmetatable(L, -2);
	return ret;
}
