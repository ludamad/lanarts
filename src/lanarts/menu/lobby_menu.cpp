/*
 * lobby_menu.cpp:
 *  Represents a server's connection lobby screen
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

#include <common/Timer.h>

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

#include "menus.h"

static void start_game_with_current_players(GameState* gs, GameInst* _,
		void* flag) {
	*(bool*)flag = true;
	gs->game_settings().conntype = GameSettings::SERVER;
}

static void draw_connectors(GameState* gs, GameInst* inst, void* flag) {
	static Colour colours[] = { COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
			COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY };
	static const int num_colours = sizeof(colours) / sizeof(Colour);

	PlayerData& pd = gs->player_data();
	std::vector<PlayerDataEntry>& pdes = pd.all_players();

	gl_printf_x_centered(gs->menu_font(), COL_WHITE, inst->x, inst->y,
			"Players in Game:");

	for (int i = 0; i < pdes.size(); i++) {
		Colour& colour = colours[i % num_colours];
		const char* prefix = i == 0 ? "You, " : "";
		const char* name = pdes[i].player_name.c_str();
		const std::string& classname =
				game_class_data.at(pdes[i].classtype).name;
		gl_printf_x_centered(gs->primary_font(), colour, inst->x,
				inst->y + 20 * i + 30, "Player %d: %s%s the %s", (i + 1),
				prefix, name, classname.c_str());
	}
}
static void setup_connectionmenu_buttons(GameState* gs, bool* exit, int x,
		int y) {
	ObjCallback single(start_game_with_current_players, exit);
	gs->add_instance(
			new DrawCallbackInst(Pos(x, y), ObjCallback(),
					ObjCallback(draw_connectors)));
	y += 200;
	gs->add_instance(new ButtonInst("Start", -1, x, y, single));
}

void lobby_menu(GameState* gs, int width, int height) {
	bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(-1, width, height));
	gs->view().x = 0;
	gs->view().y = 0;

	gs->add_instance(
			new AnimatedInst(Pos(halfw, 100), get_sprite_by_name("logo")));
	setup_connectionmenu_buttons(gs, &exit, halfw, 300);

	for (; !exit;) {
		if (!gs->update_iostate()) {
			::exit(0);
		}
		gs->net_connection().poll_messages();
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;
}
