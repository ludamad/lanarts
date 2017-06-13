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

using namespace std;

extern "C" {
// From dependency lpeg, lpeg bindings for moonscript:
int luaopen_lpeg(lua_State* L);
}

// This is a stop-gap measure to allow Lua all-or-nothing control over internal graphic initialization.
// Long-term we must move eg fonts completely to Lua.
static int __initialize_internal_graphics(lua_State* L) {
	static bool called = false;
	if (called) {
		return 0;
	}
	called = true;
	GameSettings& settings = lua_api::gamestate(L)->game_settings();
	res::font_primary().initialize(settings.font, 10);
	res::font_menu().initialize(settings.menu_font, 20);
	return 0;
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
}
#else
static void lua_vm_configure(lua_State* L) {
	luawrap::globals(L)["__LUAJIT"] = false;
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

const char* traceback(lua_State* L) {
    luawrap::globals(L)["debug"]["traceback"].push();
    lua_call(L, 0, 1);
    return lua_tostring(L, -1);
}

static shared_ptr<GameState> init_gamestate() {
    lua_State* L = init_luastate();
	GameSettings settings; // Initialized with defaults
	// Load the manual settings
	if (!load_settings_data(settings, "settings.yaml")) {
		fatal_error("Fatal error: settings.yaml not found, the game is probably being loaded from the wrong place.\n");
	}
        // Override with remembered settings
        load_settings_data(settings, "saves/saved_settings.yaml"); 
	bool can_create_saves = ensure_directory("saves");
	if (!can_create_saves) {
		printf("Problem creating save directory, will not be able to create save files!\n");
	}

        // Width 0 resolves to monitor width:
        Size screen_size = ldraw::screen_size();
        if (settings.view_width == 0) {
            settings.view_width = screen_size.w;
        }
        // Height 0 resolves to monitor height:
        if (settings.view_height == 0) {
            settings.view_height = screen_size.h;
        }

	lanarts_net_init(true);
	lsound::init();

	//GameState claims ownership of the passed lua_State*
	GameState* gs = new GameState(settings, L);
	lua_api::register_api(gs, L);

	luawrap::globals(L)["__initialize_internal_graphics"].bind_function(__initialize_internal_graphics);
	luawrap::globals(L)["_lanarts_unit_tests"].bind_function(run_unittests);

	return shared_ptr<GameState>(gs);
}

static void run_bare_lua_state(int argc, char** argv) {
    lua_State* L = init_luastate();

    if (std::getenv("LANARTS_HEADLESS") == NULL) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
                printf("SDL_Init failed: %s\n", SDL_GetError());
                exit(1);
        }
    }

        // When running a bare state, pass an empty GameSettings object:
	GameState* gs = new GameState(GameSettings(), L);
        // Create a game state, mainly for the IO state manipulation functions:
	lua_api::register_api(gs, L);

	LuaValue main_func = luawrap::dofile(L, "Main2.lua");
    if (!main_func.isnil()) {
        main_func.push();
        luawrap::call<void>(L, vector<string>(argv + 1, argv + argc));
    }
    // Cleanup
    delete gs;
    lanarts_system_quit();
}

// Returns: Should we keep running our outer game loop?
static bool run_game_instance(const vector<string>& args) {
    shared_ptr<GameState> gs = init_gamestate();

    /* Load low-level Lua bootstrapping code.
     * Implements the module system used by the rest of the engine,
     * and other important utilities.
     */
    LuaValue entry_point = luawrap::dofile(gs->luastate(), "Main.lua");

    LuaValue engine = luawrap::globals(gs->luastate())["Engine"];

    bool did_exit, should_continue;

    entry_point.push();
    should_continue = luawrap::call<bool>(gs->luastate(), args);
    if (!should_continue) {
        /* User has quit! */
        return false;
    }

    engine["menu_start"].push();
    did_exit = !luawrap::call<bool>(gs->luastate());
    save_settings_data(gs->game_settings(), "saves/saved_settings.yaml"); // Save settings from menu
    if (did_exit) {
        /* User has quit! */
        return false;
    }

    try {
        gs->start_connection();
    } catch (const LNetConnectionError& err) {
        fprintf(stderr, "The connection attempt was aborted%s\n",
                        err.what());
        return true; // Restart on connection abort TODO make this smoother
    }

    init_game_data(gs->luastate());
    engine["resources_load"].push();
    luawrap::call<void>(gs->luastate());

    if (gs->game_settings().conntype == GameSettings::SERVER) {
        engine["pregame_menu_start"].push();
        bool did_exit = !luawrap::call<bool>(gs->luastate());

        if (did_exit) { /* User has quit! */
            return false;
        }
    }

    if (gs->start_game()) {
        try {
            engine["game_start"].push();
            luawrap::call<void>(gs->luastate());
        } catch (const LNetConnectionError& err) {
            fprintf(stderr, "The game must end due to a connection termination:%s\n",
                    err.what());
        }

        if (!gs->io_controller().user_has_exit()) {
            if (gs->game_settings().conntype != GameSettings::CLIENT) {
                save_settings_data(gs->game_settings(),
                        "saves/saved_settings.yaml"); // Save settings from in-game
            }
            return true; // Keep looping
        }
    }

    if (gs->game_settings().conntype != GameSettings::CLIENT) {
        save_settings_data(gs->game_settings(), "saves/saved_settings.yaml"); // Save settings from in-game
    }
    return false; // Game over
}

// Run the loop that completely restarts the game (creating a new GameState) when the user fully exits
static void run_outer_game_loop(const vector<string>& args) {
    // Initialize SDL if we are not in headless mode
    if (std::getenv("LANARTS_HEADLESS") == NULL) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0) {
            printf("SDL_Init failed: %s\n", SDL_GetError());
            exit(1);
        }
        if (SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt") < 0) {
            printf("WARNING Controller mapping failed: %s\n", SDL_GetError());
        }
    }
    // Run game instances until the user asks to quit
    bool should_continue = true;
    while (should_continue) {
        should_continue = run_game_instance(args);
        if (std::getenv("LANARTS_HARD_QUIT") != NULL) { // TODO hack!!
            should_continue = false;
        }
    }

    lanarts_system_quit();
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
    if (argc >= 2 && std::string(argv[1]) == "bare") { // TODO, stop-gap measure until better solution
        run_bare_lua_state(argc - 1, argv + 1); // Remove 'bare' argument
    } else {
        run_outer_game_loop(vector<string>{argv + 1, argv + argc});
    }
#if NDEBUG
	} catch (const std::exception& err) {
		fprintf(stderr, "%s\n", err.what());
		fflush(stderr);
	}
#endif

	return 0;
}
