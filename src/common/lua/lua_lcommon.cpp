/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */

#include <SLB/Manager.hpp>
#include <SLB/Table.hpp>

#include "../fatal_error.h"

#include "lua_timer.h"
#include "LuaValue.h"

static int lua_error_handler(lua_State* L) {
	int nargs = lua_gettop(L);
	std::string failmsg = "An error occurred in lua's runtime:\n";

	for (int i = 0; i < nargs; i++) {
		failmsg += lua_tostring(L, i);
		failmsg += '\n';
	}
	printf(failmsg.c_str());

	fatal_error();

	return 0;
}

void lua_safe_dostring(lua_State* L, const char* code) {
	int stackn = lua_gettop(L);
	if (luaL_dostring(L, code)) {
		std::string failmsg;

		failmsg += "\nWhen running ... \n";
		failmsg += code;
		failmsg += "\n... an error occurred in lua's runtime:\n";

		failmsg += lua_tostring(L, -1);
		printf(failmsg.c_str());
		fatal_error();
	}
}

void lua_safe_dofile(lua_State* L, const char* fname) {
	if (luaL_loadfile(L, fname) != 0) {
		printf("Fatal error when loading lua file %s\n", fname);
		exit(0);
	}
	if (lua_pcall(L, 0, 0, 0)) {
		printf("Fatal error when running lua file %s:\n%s\n", fname, lua_tostring(L, -1));
		fatal_error();
	}

}

// Partially copied from Lua source
static int lua_safe_dofile(lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	lua_safe_dofile(L, fname);
	return 0;
}

void lua_register_lcommon(lua_State* L, const LuaValue& module) {
	lua_register_timer(L, module);
	lua_pushcfunction(L, lua_safe_dofile);
	lua_setglobal(L, "dofile");
}

