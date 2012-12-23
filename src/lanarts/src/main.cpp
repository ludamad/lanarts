#include <iostream>
#include <vector>
#include <ctime>

#include <SDL.h>
#include <SDL_opengl.h>

#include <ldraw/display.h>
#include <ldraw/lua_ldraw.h>
#include <net/lanarts_net.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/calls.h>

#include <lcommon/Timer.h>

#include "data/game_data.h"

#include "display/display.h"

#include "gamestate/GameState.h"

#include "interface/ButtonInst.h"

#include "lua/lua_api.h"
#include "lua/lua_yaml.h"
#include "lua_api/lua_newapi.h"

#include "objects/enemy/EnemyInst.h"

#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"

#include "../tests/tests.h"

#include "menu/menus.h"

using namespace std;

static void init_system(GameSettings& settings, lua_State* L) {
	load_settings_data(settings, "settings.yaml"); // Load the initial settings

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(0);
	}

	ldraw::display_initialize("Lanarts",
			Dim(settings.view_width, settings.view_height),
			settings.fullscreen);

	lanarts_net_init(true);
	lua_api::preinit_state(L);

	init_game_data(settings, L);

}

extern "C" {
int main(int argc, char** argv) {
	const int HUD_WIDTH = 160;
	lua_State* L = lua_open();

	GameSettings settings;
	init_system(settings, L);

	//GameState claims ownership of the passed lua_State*
	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww - HUD_WIDTH, viewh = windowh;
	GameState* gs = new GameState(settings, L, vieww, viewh);
	lua_api::register_api(gs, L);

	gs->update_iostate(); //for first iteration
	int exitcode = main_menu(gs, windoww, windowh);

	if (exitcode == 0) {
		gs->start_game();
		lua_api::luacall_main(L);
		fflush(stdout);
	}

	save_settings_data(gs->game_settings(), "saved_settings.yaml");
	SDL_Quit();

	delete gs;

//	lua_close(L); // TODO: To exit cleanly we must clear all resource vectors explicitly

	return 0;
}
}
