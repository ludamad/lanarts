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

#include "lua_api.h"


static int engine_initialize_subsystems(lua_State *L) {
    lanarts_net_init(true);
    lsound::init();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }
    if (SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt") < 0) {
        printf("WARNING Controller mapping failed: %s\n", SDL_GetError());
    }
    return 0;
}

static void engine_init_gamestate(LuaValue lsettings) {
    lua_State* L = lsettings.luastate();
    GameSettings settings; // Initialized with defaults
    settings.parse(lsettings);
    bool can_create_saves = ensure_directory("saves");
    if (!can_create_saves) {
        printf("Problem creating save directory, will not be able to create save files!\n");
    }
    //GameState claims ownership of the passed lua_State*
    GameState* gs = new GameState(settings, L);
    lua_api::register_api(gs, L);

    // Initialize fonts specified in settings
    res::font_primary().initialize(settings.font, 10);
    res::font_menu().initialize(settings.menu_font, 20);
    gs->start_game();
}

static int engine_start_game(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    gs->start_game();
    return 0;
}

// Note: After calling delete_gamestate, we generally need a new Lua state.
// This is due to 1<->1 hardcoded design between GameState's and lua_State's.
// This can be fixed by properly flushing out the lua_State data.
static int engine_deinit_gamestate(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    delete gs;
    lua_api::register_gamestate(nullptr, L);
    return 0;
}


int init_resource_data(lua_State *L);

namespace lua_api {
    int read_eval_print(lua_State *L);
	void register_lua_core_EngineInternal(lua_State* L) {
		LuaValue engine = register_lua_submodule(L, "core.EngineInternal");
        engine["init_subsystems"].bind_function(engine_initialize_subsystems);
        engine["init_resource_data"].bind_function(init_resource_data);
        engine["run_unittests"].bind_function(run_unittests);
        engine["read_eval_print"].bind_function(read_eval_print);
        engine["init_gamestate"].bind_function(engine_init_gamestate);
        engine["deinit_gamestate"].bind_function(engine_deinit_gamestate);
        engine["start_game"].bind_function(engine_start_game);
	}
}
