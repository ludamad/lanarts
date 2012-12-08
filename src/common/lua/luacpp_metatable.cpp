/*
 * luacpp_metatable.cpp:
 *  Helpers for creating a flexible metatable for lua classes holding C++ data
 *  Relies on luacpp_push, luacpp_get, luacpp_check
 */

#include "../lcommon_assert.h"

#include "luacpp_metatable.h"

// Takes obj, key
static int luameta_getmemb(lua_State* L) {
	LCOMMON_ASSERT(lua_gettop(L) == 2);
	int starttop = lua_gettop(L);
	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1));

	lua_CFunction lfunc = lua_tocfunction(L, -1);
	lua_pop(L, 1);

	if (lfunc == NULL) {
		lua_rawget(L, lua_upvalueindex(2));
		if (!lua_isnil(L, -1)) {
			return 1;
		}
		lua_pop(L, 1);
		LuaValue tbl(L, lua_upvalueindex(1));
		std::string name = tbl.get(L, "__typename");
		return luaL_error(L, "Type '%s' does not have a '%s' member!\n",
				name.c_str(), lua_tostring(L, 2));
	}

	LCOMMON_ASSERT(lua_gettop(L) == starttop);

	return lfunc(L);
}

// Takes obj, key, val
static int luameta_setmemb(lua_State* L) {
	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1));
	lua_CFunction lfunc = lua_tocfunction(L, -1);
	lua_pop(L, 1);

	if (lfunc == NULL) {
		LuaValue tbl(L, lua_upvalueindex(1));
		std::string name = tbl.get(L, "__typename");
		return luaL_error(L,
				"Type '%s' does not have a '%s' member, or it is read-only!\n",
				name.c_str(), lua_tostring(L, 2));
	}

	return lfunc(L);
}

LuaValue luameta_new(lua_State* L, const char* classname) {
	int pretop = lua_gettop(L);
	LuaValue consttable;
	consttable.table_initialize(L);

	LuaValue metatable;
	metatable.table_initialize(L);

	{ // Set up 'getter' lookup, falls back to 'consttable'
		LuaValue gettertable;
		gettertable.table_initialize(L);
		gettertable.get(L, "__typename") = classname;

		gettertable.push(L);
		consttable.push(L);
		lua_pushcclosure(L, luameta_getmemb, 2);
		metatable.get(L, "__index").pop();

		metatable.get(L, "__getters") = gettertable;
	}

	{ // Set up 'setter' function table
		LuaValue settertable;
		settertable.table_initialize(L);
		settertable.get(L, "__typename") = classname;

		settertable.push(L);
		lua_pushcclosure(L, luameta_setmemb, 1);
		metatable.get(L, "__newindex").pop();

		metatable.get(L, "__setters") = settertable;
	}

	consttable.push(L); // Set consttable as its own metatable
	consttable.push(L); // So that the __index operations chain
	lua_setmetatable(L, -2);
	lua_pop(L, 1);
	/*pop metatable*/

	LCOMMON_ASSERT(lua_gettop(L) == pretop);

	return metatable;
}

void luameta_pop_getter(lua_State* L, const LuaValue& value,
		const char* keyname) {
	LuaValue gettertable = value.get(L, "__getters");
	gettertable.get(L, keyname).pop();
}

void luameta_pop_setter(lua_State* L, const LuaValue& value,
		const char* keyname) {
	LuaValue settertable = value.get(L, "__setters");
	settertable.get(L, keyname).pop();
}

void luameta_pushcached(lua_State* L, luameta_initializer initfunc) {
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

void luameta_pop_method(lua_State* L, const LuaValue& value,
		const char* keyname) {
	int ntop = lua_gettop(L);
	LuaValue consttable = value.get(L, "__index");
	consttable.get(L, keyname).pop();
}

