/*
 * lua_utils.cpp:
 *  Various lua utilities.
 */

#include "lua_utils.h"

#include <lua.hpp>

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
	return val;
}

void lua_protect_table(const LuaField& field) {
	lua_State* L = field.luastate();

	field.push();
	luameta_push(L, create_protection_table);
	lua_setmetatable(L, -2);
	lua_pop(L, 1); /* Pop field */
}
