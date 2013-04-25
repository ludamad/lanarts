/*
 * lanarts_examples.cpp:
 *  Exercises some of the hard-to-unit test code
 */

#include <cstdio>

#include <lua.hpp>

#include <lcommon/math_util.h>

#include <luawrap/luawrap.h>

#include <lcommon/lua_lcommon.h>
#include <lcommon/fatal_error.h>

#include "gamestate/GameState.h"

#include "lua_api/lua_newapi.h"

int read_eval_print(lua_State *L);

static void perform_luascript(lua_State* L, const char* file) {
	luawrap::dofile(L, file);
	lua_api::globals_set_mutability(L, false);

	luawrap::globals(L)["main"].push();
	if (lua_isnil(L, -1)) {
		luawrap::call<void>(L);
	}

	if (lua_tostring(L,-1)) {
		printf("%s\n", lua_tostring(L,-1));
	}
}

static lua_State* setup_lua_state() {
	using namespace ldraw;

	lua_State* L = lua_api::create_luastate();
	lua_api::add_search_path(L, "res/?.lua");

	lua_api::register_api(new GameState(GameSettings(), L), L);

	LuaValue globals(L, LUA_GLOBALSINDEX);

	// Expose some additional functions only for the stand-alone runner
	globals["read_eval_print"].bind_function(read_eval_print); // Blocks until one command is entered

	return L;
}

// Must be char** argv to play nice with SDL on windows!
int main(int argc, char** argv) {
	try {
		using namespace ldraw;

		lua_State* L = setup_lua_state();

		if (argc < 2) {
			printf("Welcome to LanartsEngine, please type some Lua\n");
			while (true) {
				luawrap::push_function(L, read_eval_print);
				luawrap::call<void>(L);
			}
		} else {
			perform_luascript(L, argv[1]);
		}

		lua_close(L);
	} catch (const __FatalError& err) {
		printf("Fatal error occurred in lanarts runner, exiting ... \n");
	}
}
