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

static void engine_init_gamestate(LuaValue lsettings) {
    lua_State* L = lsettings.luastate();
    GameSettings settings; // Initialized with defaults
    bool can_create_saves = ensure_directory("saves");
    if (!can_create_saves) {
        printf("Problem creating save directory, will not be able to create save files!\n");
    }

    lanarts_net_init(true);
    lsound::init();

    //GameState claims ownership of the passed lua_State*
    GameState* gs = new GameState(settings, L);
    lua_api::register_api(gs, L);
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

namespace lua_api {
    int read_eval_print(lua_State *L);
	void register_lua_core_EngineInternal(lua_State* L) {
		LuaValue engine = register_lua_submodule(L, "core.EngineInternal");
        engine["initialize_internal_graphics"].bind_function(__initialize_internal_graphics);
        engine["lanarts_unit_tests"].bind_function(run_unittests);
        engine["read_eval_print"].bind_function(read_eval_print);
        engine["init_gamestate"].bind_function(engine_init_gamestate);
        engine["deinit_gamestate"].bind_function(engine_deinit_gamestate);
	}
}
