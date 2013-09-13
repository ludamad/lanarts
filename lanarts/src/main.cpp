#include <iostream>
#include <vector>
#include <stdexcept>
#include <ctime>

#include <lcommon/fatal_error.h>

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
#include "lua_api/lua_newapi.h"

// This is a stop-gap measure to allow Lua all-or-nothing control over internal graphic initialization.
// Long-term we must move eg fonts completely to Lua.
static int __initialize_internal_graphics(lua_State* L) {
	GameSettings& settings = lua_api::gamestate(L)->game_settings();
	res::font_primary().initialize(settings.font, 10);
	res::font_menu().initialize(settings.menu_font, 20);
	return 0;
}

static GameState* init_gamestate() {

	lua_State* L = lua_api::create_configured_luastate();
	lua_api::add_search_path(L, "modules/lanarts/?.lua");

	GameSettings settings; // Initialized with defaults
	// Load the manual settings
	if (!load_settings_data(settings, "settings.yaml")) {
		fatal_error("Fatal error: settings.yaml not found, the game is probably being loaded from the wrong place.\n");
	}

	bool can_create_saves = ensure_directory("saves");
	if (!can_create_saves) {
		printf("Problem creating save directory, will not be able to create save files!\n");
	}
	load_settings_data(settings, "saves/saved_settings.yaml"); // Override with remembered settings

	if (SDL_Init(0) < 0) {
		exit(0);
	}

	lanarts_net_init(true);
	lsound::init();

	//GameState claims ownership of the passed lua_State*
	GameState* gs = new GameState(settings, L);
	lua_api::register_api(gs, L);

	luawrap::globals(L)["__initialize_internal_graphics"].bind_function(__initialize_internal_graphics);

	return gs;
}

static int run_engine(int argc, char** argv) {
	label_StartOver:

	GameState* gs = init_gamestate();
	lua_State* L = gs->luastate();

	/* Load low-level Lua bootstrapping code.
	 * Implements the module system used by the rest of the engine.
	 */
	luawrap::dofile(L, "modules/Main.lua");

	LuaValue engine = luawrap::globals(L)["Engine"];

	bool did_exit, should_continue;

	engine["main"].push();
	should_continue = luawrap::call<bool>(L,
			std::vector<std::string>(argv + 1, argv + argc));
	if (!should_continue) {
		/* User has quit! */
		goto label_Quit;
	}

	engine["menu_start"].push();
	did_exit = !luawrap::call<bool>(L);
	save_settings_data(gs->game_settings(), "saves/saved_settings.yaml"); // Save settings from menu
	if (did_exit) {
		/* User has quit! */
		goto label_Quit;
	}

	gs->start_connection();

	engine["resources_load"].push();
	luawrap::call<void>(L);

	try {
		init_game_data(gs->game_settings(), L);
		engine["resources_post_load"].push();
		luawrap::call<void>(L);
	} catch (const std::exception& err) {
		fprintf(stderr, "%s\n", err.what());
		fflush(stderr);
		goto label_Quit;
	}

	if (gs->game_settings().conntype == GameSettings::SERVER) {
		engine["pregame_menu_start"].push();
		bool did_exit = !luawrap::call<bool>(L);

		if (did_exit) { /* User has quit! */
			goto label_Quit;
		}
	}

	if (gs->start_game()) {
		try {
			engine["game_start"].push();
			luawrap::call<void>(L);
		} catch (const LNetConnectionError& err) {
			fprintf(stderr, "The game must end due to a connection termination:%s\n",
					err.what());
		}

		if (!gs->io_controller().user_has_exit()) {
			if (gs->game_settings().conntype != GameSettings::CLIENT) {
				save_settings_data(gs->game_settings(),
						"saves/saved_settings.yaml"); // Save settings from in-game
			}
			delete gs;
			goto label_StartOver;
		}
	}

	if (gs->game_settings().conntype != GameSettings::CLIENT) {
		save_settings_data(gs->game_settings(), "saves/saved_settings.yaml"); // Save settings from in-game
	}

	label_Quit:

	lanarts_system_quit();

	delete gs;

	printf("Lanarts shut down cleanly\n");

//	lua_close(L); // TODO: To exit cleanly we must clear all resource vectors explicitly

	return 0;
}

/* Must take (int, char**) to play nice with SDL */
int main(int argc, char** argv) {
	try {
		run_engine(argc, argv);
	} catch (const std::exception& err) {
		fprintf(stderr, "%s\n", err.what());
		fflush(stderr);
	}
}
