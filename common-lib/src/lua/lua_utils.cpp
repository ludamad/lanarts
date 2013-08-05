/*
 * lua_utils.cpp:
 *  Various lua utilities.
 */

#include "lua_utils.h"

#include <lua.hpp>

#include "lua_serialize.h"

#include "strformat.h"

#include <luawrap/luawrap.h>
#include <luawrap/luameta.h>

static int error_fallback(lua_State* L) {
	const char* key = lua_tostring(L, 2);
	luawrap::push(L, format("Key '%s' does not exist in table!", key));
	return lua_error(L);
}

static LuaValue create_protection_table(lua_State* L) {
	LuaValue val(L);
	val.newtable();
	val["__index"].bind_function(error_fallback);
	lua_register_for_serialization("C;lua_utils;__index", val["__index"]);
	return val;
}

/* Protect a lua table from returning nil. */
void lua_protect_table(const LuaField& field) {
	lua_State* L = field.luastate();

	field.push();
	luameta_push(L, create_protection_table);
	lua_setmetatable(L, -2);
	lua_pop(L, 1); /* Pop field */
}

/* Ensures a table exists here, and create a protected table if not */
LuaValue lua_ensure_protected_table(const LuaField& field) {
	if (!field.isnil()) {
		return field;
	}
	LuaValue table = luawrap::ensure_table(field);
	lua_protect_table(table);
	return table;
}
