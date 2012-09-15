/*
 * start_menu.cpp:
 *  First screen seen
 */

#include <iostream>
#include <vector>
#include <ctime>

#include <SDL.h>
#include <SDL_opengl.h>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../data/game_data.h"

#include "../display/colour_constants.h"
#include "../display/display.h"

#include "../gamestate/GameState.h"

#include "../interface/ButtonInst.h"
#include "../interface/TextBoxInst.h"

#include "../interface/DrawCallbackInst.h"
#include "../lua/lua_api.h"

#include "../objects/enemy/EnemyInst.h"
#include "../objects/player/PlayerInst.h"
#include "../objects/AnimatedInst.h"

#include "../util/Timer.h"

#include "menus.h"

static void continue_as_loner(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().regen_on_death = true;
	gs->game_settings().frame_action_repeat = 0;
}

static void continue_as_loner_save_replay(GameState* gs, GameInst* _,
		void* flag) {
	*(bool*)flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().savereplay_file = "replays/replay";
	gs->game_settings().frame_action_repeat = 0;
}

static void continue_as_load_replay(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().loadreplay_file = "replays/replay";
	gs->game_settings().frame_action_repeat = 0;
}

static void continue_as_client(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().conntype = GameSettings::CLIENT;
}
static void continue_as_server(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().conntype = GameSettings::SERVER;
}
static const char HELP_TEXT[] = "Movement: WASD or Arrow Keys\n"
		"Switch Targetted Enemy: k\n"
		"Use Weapon: h \n"
		"Use Current Spell/Weapon: hold j or space or left click\n"
		"Switch Spell: q/e, mousewheel to toggle, y, u, i, o, p to choose \n"
		"Use Specific Spell: y, u, i, o, p once to choose, repeat to fire\n"
		"Use Item: click item or keys 1 through 9\n"
		"Use Stairs: Move onto them without holding other keys\n";

static void setup_mainmenu_buttons(GameState* gs, bool* exit, int x, int y) {
	ObjCallback single(continue_as_loner, exit);
	ObjCallback savereplay(continue_as_loner_save_replay, exit);
	ObjCallback loadreplay(continue_as_load_replay, exit);
	ObjCallback client(continue_as_client, exit);
	ObjCallback server(continue_as_server, exit);
	y += 50;
	gs->add_instance(new ButtonInst("Start", -1, x, y, single));
//	gs->add_instance(new ButtonInst("Save Replay", -1, x - 95, y, savereplay));
//	gs->add_instance(new ButtonInst("Load Replay", -1, x + 95, y, loadreplay));
//	y += 50;
//	gs->add_instance(new ButtonInst("Client", -1, x - 65, y, client));
//	gs->add_instance(new ButtonInst("Server", -1, x + 65, y, server));
//	y += 50;
}

void main_menu(GameState* gs, int width, int height) {
	setup_menu_again: bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(-1, width, height));
	gs->view().x = 0;
	gs->view().y = 0;

	gs->add_instance(
			new AnimatedInst(Pos(halfw, 100), get_sprite_by_name("logo")));
	gs->add_instance(
			animated_inst(Pos(halfw - 100, 460), HELP_TEXT,
					Colour(255, 255, 255)));
	setup_mainmenu_buttons(gs, &exit, halfw, 300);

	for (; !exit && !gs->key_press_state(SDLK_RETURN);) {
		if (!gs->update_iostate()) {
			::exit(0);
		}
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;

	int exitcode = class_menu(gs, width, height);
	if (exitcode < 0) {
		goto setup_menu_again;
	}
	gs->start_connection();

	if (gs->game_settings().conntype == GameSettings::SERVER) {
		lobby_menu(gs, width, height);
	}
}

