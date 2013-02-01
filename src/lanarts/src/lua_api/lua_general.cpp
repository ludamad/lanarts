/*
 * lua_general.cpp:
 *  General utility functions.
 */

#include <cstring>

#include <lua.hpp>

#include <lcommon/mathutil.h>

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include "lua_newapi.h"

#include "util/math_util.h"

static int lapi_values_aux(lua_State* L) {
	long idx = (long)lua_touserdata(L, lua_upvalueindex(2));
	long len = (long)lua_touserdata(L, lua_upvalueindex(3));

	if (idx > len) {
		return 0;
	}

	lua_pushlightuserdata(L, (void*)(1 + idx));
	lua_replace(L, lua_upvalueindex(2));

	lua_rawgeti(L, lua_upvalueindex(1), idx);
	if (lua_isnil(L, -1)) {
		luaL_error(L,
				"Expected array as parameter to 'values', but retrieved a nil value at index '%d'.",
				idx);
	}
	return 1;
}

// Try getters, then methods, then inherited __index
static int lapi_newtype_index(lua_State* L) {
	// Upvalue 1: Getters

	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1)); // Push getter

	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1); // obj
		lua_call(L, 1, 1); // Call getter with obj
		return 1;
	}

	// Upvalue 2: Metatable

	lua_pushvalue(L, 2); // key
	lua_rawget(L, lua_upvalueindex(2)); // metatable table

	if (!lua_isnil(L, -1)) {
		return 1;
	}

	// Upvalue 3: Inherited metatable

	if (!lua_isnil(L, lua_upvalueindex(3))) {
		lua_getfield(L, lua_upvalueindex(3), "__index");
		lua_pushvalue(L, 1); // obj
		lua_pushvalue(L, 2); // key
		lua_call(L, 2, 1);
		return 1;
	}

	LuaSpecialValue metatable(L, lua_upvalueindex(2));
	metatable["typename"].push();

	return luaL_error(L,
			"Type '%s': Cannot read '%s', member does not exist!\n",
			lua_tostring(L, -1), lua_tostring(L, 2));
}

static int lapi_newtype_newindex(lua_State* L) {
	// Upvalue 1: Setters

	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1)); // Push setter

	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1); // obj
		lua_pushvalue(L, 3); // value
		lua_call(L, 2, 0); // Call setter with obj, value
		return 0;
	}
//
//	// Upvalue 2: Inherited metatable

	if (!lua_isnil(L, lua_upvalueindex(2))) {
		lua_getfield(L, lua_upvalueindex(2), "__newindex");
		lua_pushvalue(L, 1); // obj
		lua_pushvalue(L, 2); // key
		lua_pushvalue(L, 3); // value
		lua_call(L, 3, 0);
		return 0;
	}

	// Try object itself:
	lua_settop(L, 3);
	lua_rawset(L, 1);
	return 0;
}

static int lapi_newtype_create(lua_State* L) {
	int nargs = lua_gettop(L);

	lua_newtable(L);

	LuaSpecialValue metatable(L, lua_upvalueindex(1));
	metatable["init"].push(); // push the init function

	lua_pushvalue(L, -2); // push the new instance as 1st arg

	metatable.push();
	lua_setmetatable(L, -2); // set the new instance's metatable

	if (lua_isnil(L, -2)) { // Function is nil
		return 1; // Just return new instance now
	}

	for (int i = 1; i <= nargs; i++) {
		lua_pushvalue(L, i); // Passed arguments
	}

	lua_call(L, nargs + 1, 0);

	return 1;
}

static int lapi_newtype(lua_State* L) {
	lua_newtable(L);
	// new type metatable
	LuaStackValue newtype(L, -1);

	newtype.push();
	lua_pushcclosure(L, lapi_newtype_create, 1);
	newtype["create"].pop();

	newtype["set"].ensure_table();
	newtype["get"].ensure_table();

	if (!lua_isnil(L, 1)) {
		LuaStackValue parameters(L, 1);

		parameters["parent"].push();
		lua_setmetatable(L, -2); // set new type metatable's metatable

		newtype["typename"] = parameters["typename"];
		newtype["parent"] = parameters["parent"];
	} else {
		newtype["typename"] = "<UserDefinedType>";
	}

	newtype["get"].push(); // Upvalue 1: getters
	newtype.push(); // Upvalue 2: metatable
	newtype["parent"].push(); // Upvalue 3: parent metatable
	lua_pushcclosure(L, lapi_newtype_index, 3);
	newtype["__index"].pop();

	newtype["set"].push(); // Upvalue 1: setters
	newtype["parent"].push(); // Upvalue 2: parent metatable
	lua_pushcclosure(L, lapi_newtype_newindex, 2);
	newtype["__newindex"].pop();

	return 1; // return new type metatable
}

// Assumes paths do not change root
static int lapi_import(lua_State* L) {
	lua_pushvalue(L, 1);
	lua_gettable(L, lua_upvalueindex(1));

	if (lua_isnil(L, -1)) {
		luawrap::globals(L)["dofile"].push();
		lua_pushvalue(L, 1);
		lua_call(L, 1, 0);
	}

	lua_pushvalue(L, 1);
	lua_pushboolean(L, true);
	lua_settable(L, lua_upvalueindex(1));

	return 1;
}

static int lapi_string_split(lua_State *L) {
	size_t str_size;
	const char* tail = luaL_checklstring(L, 1, &str_size);
	const char* end = tail + str_size;

	size_t sep_size = 0;
	const char* sep = luaL_checklstring(L, 2, &sep_size);
	int i = 1;

	/* push result table */
	lua_newtable(L);

	/* repeat for each separator */
	while (tail < end) {
		const char* head = tail;
		/* find separator */
		while (strncmp(head, sep, sep_size) != 0) {
			head++;
			if (head >= end) {
				goto label_LeaveLoop;
			}
		}

		lua_pushlstring(L, tail, head - tail); /* push substring */
		lua_rawseti(L, -2, i++);
		tail = head + sep_size; /* skip separator */
	}

	label_LeaveLoop:

	/* push last substring */
	lua_pushstring(L, tail);
	lua_rawseti(L, -2, i);

	return 1; /* return the table */
}

static int lapi_setglobal(lua_State *L) {
	bool prev = lua_api::globals_get_mutability(L);

	lua_api::globals_set_mutability(L, true);
	lua_settable(L, LUA_GLOBALSINDEX); // use params 1 & 2
	lua_api::globals_set_mutability(L, prev);

	return 1; /* return the table */
}

static int lapi_toaddress(lua_State *L) {
	char address[64];
	snprintf(address, 64, "0x%X", lua_topointer(L, 1));
	lua_pushstring(L, address);
	return 1; /* return the table */
}


namespace lua_api {

	int l_itervalues(lua_State* L) {
		lua_pushvalue(L, 1);
		lua_pushlightuserdata(L, (void*)(1));
		lua_pushlightuserdata(L, (void*)(lua_objlen(L, 1)));
		lua_pushcclosure(L, lapi_values_aux, 3);
		return 1;
	}

	void register_general_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		globals["values"].bind_function(l_itervalues);
		globals["direction"].bind_function(compute_direction);
		globals["distance"].bind_function(distance_between);
		globals["newtype"].bind_function(lapi_newtype);
		globals["string_split"].bind_function(lapi_string_split);
		globals["setglobal"].bind_function(lapi_setglobal);
		globals["toaddress"].bind_function(lapi_toaddress);

		lua_newtable(L);
		lua_pushcclosure(L, lapi_import, 1);
		globals["import"].pop();

		globals["system"].ensure_table();
	}
}
