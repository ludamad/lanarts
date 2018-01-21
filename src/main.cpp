#include <iostream>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <csignal>

#include <memory>

#include <lcommon/fatal_error.h>
#include <lcommon/unittest.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <lua.hpp>

#include <lcommon/directory.h>

#include <net-lib/lanarts_net.h>

#include <lsound/lsound.h>

#include <ldraw/display.h>
#include <ldraw/lua_ldraw.h>

#include <luawrap/luawrap.h>

#include "data/game_data.h"

#include "draw/draw_sprite.h"
#include "draw/fonts.h"

#include "gamestate/GameState.h"

#include "lua_api/lua_api.h"
#include "lua_api/lua_yaml.h"
#include "lua_api/lua_api.h"
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

using namespace std;

extern "C" {
// From dependency lpeg, lpeg bindings for moonscript:
int luaopen_lpeg(lua_State* L);
}

#ifdef USE_LUAJIT

extern "C" {
#include <luajit.h>
}

// LuaJIT only: Catch C++ exceptions and convert them to Lua error messages.
static int luajit_wrap_exceptions(lua_State *L, lua_CFunction f) {
	try {
		return f(L);  // Call wrapped function and return result.
	} catch (const char *s) {  // Catch and convert exceptions.
		lua_pushstring(L, s);
	} catch (const std::exception& e) {
		lua_pushstring(L, e.what());
	}
	return lua_error(L);  // Rethrow as a Lua error.
}

static void lua_vm_configure(lua_State* L) {
	lua_pushlightuserdata(L, (void *)luajit_wrap_exceptions);
	luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC|LUAJIT_MODE_ON);
	lua_pop(L, 1);
	luawrap::globals(L)["__LUAJIT"] = true;
    luawrap::globals(L)["__EMSCRIPTEN"] = false;
}
#else
static void lua_vm_configure(lua_State* L) {
	luawrap::globals(L)["__LUAJIT"] = false;
#ifdef __EMSCRIPTEN__
    luawrap::globals(L)["__EMSCRIPTEN"] = true;
#else
    luawrap::globals(L)["__EMSCRIPTEN"] = false;
#endif
}
#endif

static lua_State* init_luastate() {
	lua_State* L = lua_api::create_configured_luastate();
	lua_vm_configure(L);
	lua_api::add_search_path(L, "?.lua");
    // Open lpeg first as the moonscript library depends on lpeg, and the moonscript library is called during error reporting.
    luaopen_lpeg(L);
    lua_api::register_lua_libraries(L);
    return L;
}

// For gdb
const char* ltraceback(lua_State* L) {
    luawrap::globals(L)["debug"]["traceback"].push();
    lua_call(L, 0, 1);
    return lua_tostring(L, -1);
}

static void run_lua_value(void* arg) {
    try {
        auto& value = *(LuaValue*)arg;
        value.push();
        luawrap::call<void>(value.luastate());
    } catch (const std::exception& err) {
        fprintf(stderr, "%s\n", err.what());
        fflush(stderr);
    }
}


static void run_engine_Main(int argc, const char **argv) {
    lua_State* L = init_luastate();

    // Lua code uses lua_core_EngineInternal.cpp to do low-level setup
	LuaValue main_func = luawrap::dofile(L, "engine/Main.lua");
    if (main_func.isnil()) {
        printf("Expected main function returned from engine/Main.lua!\n");
        exit(1);
    }
    main_func.push();
    // Call with args, ignoring arg 0 (location to executable)
    LuaValue step_func = luawrap::call<LuaValue>(L, vector<string>(argv + 1, argv + argc));
    if (step_func.isnil()) {
        return;
    }
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(run_lua_value, (void*)new LuaValue(step_func), 0, 0);
#else
    while (true) {
        step_func.push();
        if (!luawrap::call<bool>(L)) {
            break;
        }
    }
#endif
}

/* Handle Ctrl+C */
static void handleSIGINT(int signal) {
  lanarts_system_quit();
  exit(0);
}

#ifdef main
# undef main
#endif /* main */
/* Must take (int, char**) to play nice with SDL */
int main(int argc, char** argv) {
  std::signal(SIGINT, handleSIGINT);
#if NDEBUG
	try {
#endif
#ifdef __EMSCRIPTEN__
    printf("WHATSUP\n");
    vector<const char*> args = {"./lanarts", "engine.StartLanarts"};
    run_engine_Main(args.size(), &args[0]); // Remove 'bare' argument
#else
    run_engine_Main(argc, (const char**)argv); // Remove 'bare' argument
#endif
#if NDEBUG
	} catch (const std::exception& err) {
		fprintf(stderr, "%s\n", err.what());
		fflush(stderr);
	}
#endif

	return 0;
}
