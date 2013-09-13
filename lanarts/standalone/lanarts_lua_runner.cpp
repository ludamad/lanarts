/*
 * lanarts_lua_runner.cpp:
 *  Lanarts engine lua script runner.
 */

#include <cstdio>

#include <lua.hpp>

#include <lcommon/math_util.h>

#include <luawrap/luawrap.h>

#include <ldraw/display.h>

#include <lcommon/lua_lcommon.h>
#include <lcommon/fatal_error.h>

#include "gamestate/GameState.h"

#include "lua_api/lua_newapi.h"

/* Defined in lanarts_lua_repl.cpp
 * Blocks until a lua expression is received from standard input */
int read_eval_print(lua_State *L);

static lua_State* setup_lua_state() {
	using namespace ldraw;

	lua_State* L = lua_api::create_configured_luastate();

	lua_api::add_search_path(L, "modules/?.lua");
	lua_api::register_api(new GameState(GameSettings(), L), L);

	return L;
}

static void perform_luascript(lua_State* L, const char* file) {
	luawrap::dofile(L, file);
	luawrap::globals(L)["main"].push();
	if (!lua_isnil(L, -1)) {
		luawrap::call<void>(L);
	}
}

// Must be char** argv to play nice with SDL on windows!
int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(0);
	}

	lua_State* L = setup_lua_state();
	LuaValue args = luawrap::ensure_table(luawrap::globals(L)["_cmdargs"]);
	for (int i = 2; i < argc; i++) {
		args[i-1] = argv[i];
	}
	try {
		if (argc < 2) {
			printf("Welcome to LanartsEngine, please type some Lua\n");
			while (true) {
				luawrap::push_function(L, read_eval_print);
				luawrap::call<void>(L);
			}
		} else {
			ldraw::display_initialize("Lanarts Example Runner", Size(1, 1));
			perform_luascript(L, argv[1]);
		}

		lua_close(L);
	} catch (const luawrap::Error& lwrap_err) {
		std::string err = lwrap_err.what();
		luawrap::print_stacktrace(L, err);
	} catch (const __FatalError& err) {
		printf("Fatal error occurred in lanarts runner, exiting ... \n");
	}
}
