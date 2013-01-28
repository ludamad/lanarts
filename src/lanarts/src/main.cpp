#include <iostream>
#include <vector>
#include <ctime>

#include <SDL.h>
#include <SDL_opengl.h>

#include <lua.hpp>

#include <net/lanarts_net.h>
#include <lsound/lsound.h>

#include <ldraw/display.h>
#include <ldraw/lua_ldraw.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>

#include "data/game_data.h"

#include "draw/draw_sprite.h"

#include "gamestate/GameState.h"

#include "lua_api/lua_api.h"
#include "lua_api/lua_yaml.h"
#include "lua_api/lua_newapi.h"

#include "menu/menus.h"

using namespace std;

static void init_system(GameSettings& settings, lua_State* L) {
	load_settings_data(settings, "settings.yaml"); // Load the initial settings

	if (SDL_Init(0) < 0) {
		exit(0);
	}

	Dim window_size(settings.view_width, settings.view_height);
	ldraw::display_initialize("Lanarts", window_size, settings.fullscreen);

	lanarts_net_init(true);
	lua_api::preinit_state(L);
	lsound::init();

	init_game_data(settings, L);

}

int main(int argc, char** argv) {
	const int HUD_WIDTH = 160;
	lua_State* L = lua_api::create_luastate();

	GameSettings settings;
	init_system(settings, L);

	//GameState claims ownership of the passed lua_State*
	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww - HUD_WIDTH, viewh = windowh;
	GameState* gs = new GameState(settings, L, vieww, viewh);
	lua_api::register_api(gs, L);

	gs->update_iostate(); //for first iteration
	int exitcode = main_menu(gs, windoww, windowh);

	if (exitcode == 0 && gs->start_game()) {
		lua_getglobal(L, "main");
		luawrap::call<void>(L);
		fflush(stdout);
	}

	save_settings_data(gs->game_settings(), "saved_settings.yaml");

	lanarts_quit();

	delete gs;

//	lua_close(L); // TODO: To exit cleanly we must clear all resource vectors explicitly

	return 0;
}
