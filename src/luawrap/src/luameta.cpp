/*
 * luacpp_metatable.cpp:
 *  Helpers for creating a flexible metatable for lua classes holding C++ data
 *  Relies on luacpp_push, luacpp_get, luacpp_check
 */

#include <lua.hpp>

#include <luawrap/luameta.h>
#include "luawrapassert.h"

static const int LUAMETA_SETTER_FALLBACK_UPVALUE = 2;
static const int LUAMETA_GETTER_FALLBACK_UPVALUE = 3;

// Takes obj, key
static int luameta_get_member(lua_State* L) {
	LUAWRAP_ASSERT(lua_gettop(L) == 2);

	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1)); // Push getter

	// Check getter table
	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_call(L, 2, 1); // Call getter with obj, key
		return 1;
	}

	// Check constants table
	lua_pushvalue(L, 2); // key
	lua_rawget(L, lua_upvalueindex(2));

	if (!lua_isnil(L, -1)) {
		return 1;
	}

	// Fallback to default getter (if one exists)
	const int fallback_upvalue =
			lua_upvalueindex(LUAMETA_GETTER_FALLBACK_UPVALUE);
	if (!lua_isnil(L, fallback_upvalue)) {

		// If it is table, set directly, otherwise call as function (below)
		if (lua_istable(L, fallback_upvalue)) {
			lua_settop(L, 2); // NB: key is on top
			lua_gettable(L, fallback_upvalue);
			return 1;
		} else {
			lua_pushvalue(L, fallback_upvalue);
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 2);
			lua_call(L, 2, 1); // Call getter with obj, key
			return 1;

		}
	}
	// Error out!

	LuaStackValue tbl(L, lua_upvalueindex(1));
	tbl["__typename"].push();
	return luaL_error(L,
			"Type '%s': Cannot read '%s', member does not exist!\n",
			lua_tostring(L, -1), lua_tostring(L, 2));

}

// Takes obj, key, val
static int luameta_set_member(lua_State* L) {
	LUAWRAP_ASSERT(lua_gettop(L) == 3);

	lua_pushvalue(L, 2); // Push key
	lua_gettable(L, lua_upvalueindex(1)); // Push setter

	// Fallback to default setter (if one exists)

	if (lua_isnil(L, -1)
			&& !lua_isnil(L, lua_upvalueindex(LUAMETA_SETTER_FALLBACK_UPVALUE))) {
		lua_pop(L, 1);
		lua_pushvalue(L, lua_upvalueindex(LUAMETA_SETTER_FALLBACK_UPVALUE));

		// If it is table, set directly, otherwise call as function (below)
		if (lua_istable(L, -1)) {
			lua_pushvalue(L, 2);
			lua_pushvalue(L, 3);
			lua_settable(L, -3);
			return 0;
		}
	}

	if (!lua_isnil(L, -1)) {

		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_call(L, 3, 0); // Call setter with obj, key, val

		return 0;
	}

	LuaStackValue tbl(L, lua_upvalueindex(1));
	tbl["__typename"].push();
	return luaL_error(L,
			"Type '%s': Cannot write '%s' member, member does not exist, or cannot be over-written!\n",
			lua_tostring(L, -1), lua_tostring(L, 2));
}

/*
 * Creates a new metatable that has getters, setters, and constants cleanly separated.
 * Getters and setters are checked first, and then a constant is looked-up.
 */
LuaValue luameta_new(lua_State* L, const char* classname) {
	int pretop = lua_gettop(L);

	LuaValue constanttable(L);
	constanttable.newtable();

	LuaValue metatable(L);
	metatable.newtable();

	{ // Set up 'getter' lookup, falls back to 'membertable'
		LuaValue gettertable(L);
		gettertable.newtable();

		lua_pushstring(L, classname);
		gettertable["__typename"].pop();

		gettertable.push();
		constanttable.push();
		lua_pushnil(L); // No default getter
		lua_pushcclosure(L, luameta_get_member, 3);
		metatable["__index"].pop();

		metatable["__getters"] = gettertable;
	}

	{ // Set up 'setter' function table
		LuaValue settertable(L);
		settertable.newtable();

		lua_pushstring(L, classname);
		settertable["__typename"].pop();

		settertable.push();
		lua_pushnil(L); // No default setter
		lua_pushcclosure(L, luameta_set_member, 2);
		metatable["__newindex"].pop();

		metatable["__setters"] = settertable;
	}

	metatable["__constants"] = constanttable;

	LUAWRAP_ASSERT(lua_gettop(L) == pretop);

	return metatable;
}

LuaValue luameta_getters(const LuaValue& metatable) {
	return metatable["__getters"];
}

LuaValue luameta_setters(const LuaValue& metatable) {
	return metatable["__setters"];
}

LuaValue luameta_constants(const LuaValue& metatable) {
	return metatable["__constants"];
}

/*
 * Pushes cached metatable if exists, or initializes cache and then pushes.
 */
void luameta_push(lua_State* L, luameta_initializer initfunc) {
	lua_pushlightuserdata(L, (void*) initfunc);
	lua_gettable(L, LUA_REGISTRYINDEX);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		LuaValue metatable = initfunc(L);
		lua_pushlightuserdata(L, (void*) initfunc);
		metatable.push();
		lua_settable(L, LUA_REGISTRYINDEX);

		metatable.push();
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

/*
 * Install a default catch-all setter.
 */
void luameta_defaultsetter(const LuaValue& metatable, const LuaValue& setter) {
	lua_State* L = metatable.luastate();

	metatable["__newindex"].push();
	setter.push();
	lua_setupvalue(L, -2, LUAMETA_SETTER_FALLBACK_UPVALUE);

	lua_pop(L, 1);
}

/*
 * Install a default catch-all getter.
 */
void luameta_defaultgetter(const LuaValue& metatable, const LuaValue& getter) {
	lua_State* L = metatable.luastate();

	metatable["__index"].push();
	getter.push();
	lua_setupvalue(L, -2, LUAMETA_GETTER_FALLBACK_UPVALUE);

	lua_pop(L, 1);
}

/*
 * Set up the __gc hook
 */
void luameta_gc(const LuaValue& metatable, lua_CFunction func) {
	metatable["__gc"].bind_function(func);
}

LuaModule::LuaModule(lua_State* L, const char* module_name) :
				proxy(L) {
	metatable = luameta_new(L, module_name);

	// Create proxy table
	lua_newtable(L);
	metatable.push();
	lua_setmetatable(L, -2);
	proxy.pop();

	values = luameta_constants(metatable);
	setters = luameta_setters(metatable);
	getters = luameta_getters(metatable);
}

LuaModule::LuaModule() {
}
