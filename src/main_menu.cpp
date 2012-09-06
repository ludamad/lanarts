/*
 * main_menu.cpp:
 *  Implements start menu and other actions
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

#include "data/game_data.h"

#include "display/colour_constants.h"
#include "display/display.h"

#include "gamestate/GameState.h"

#include "interface/ButtonInst.h"

#include "interface/DrawCallbackInst.h"
#include "lua/lua_api.h"

#include "objects/enemy/EnemyInst.h"
#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"

#include "util/Timer.h"

static void continue_as_loner(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().regen_on_death = true;
	gs->game_settings().frame_action_repeat = 0;
}
static void continue_as_hardcore(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().regen_on_death = false;
	gs->game_settings().frame_action_repeat = 0;
}

static void continue_as_loner_save_replay(GameState* gs, GameInst* _,
		void* flag) {
	*(bool*) flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().savereplay_file = "replays/replay";
	gs->game_settings().frame_action_repeat = 0;
}

static void continue_as_load_replay(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().conntype = GameSettings::NONE;
	gs->game_settings().loadreplay_file = "replays/replay";
	gs->game_settings().frame_action_repeat = 0;
}

static void continue_as_client(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().conntype = GameSettings::CLIENT;
}

static void continue_as_server(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
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
	ObjCallback hardcoresingle(continue_as_hardcore, exit);
	ObjCallback savereplay(continue_as_loner_save_replay, exit);
	ObjCallback loadreplay(continue_as_load_replay, exit);
	ObjCallback client(continue_as_client, exit);
	ObjCallback server(continue_as_server, exit);
	gs->add_instance(new ButtonInst("Single-Player", -1, x, y, single));
	y += 50;
	gs->add_instance(
			new ButtonInst("Hardcore (No Respawn)", -1, x, y, hardcoresingle));
	y += 50;
	gs->add_instance(new ButtonInst("Save Replay", -1, x - 95, y, savereplay));
	gs->add_instance(new ButtonInst("Load Replay", -1, x + 95, y, loadreplay));
	y += 50;
	gs->add_instance(new ButtonInst("Client", -1, x - 65, y, client));
	gs->add_instance(new ButtonInst("Server", -1, x + 65, y, server));
	y += 50;
}

void connection_menu_loop(GameState* gs, int width, int height);
void class_menu_loop(GameState* gs, int width, int height);

void main_menu_loop(GameState* gs, int width, int height) {
	bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(-1, width, height));
	gs->view().x = 0;
	gs->view().y = 0;

	gs->add_instance(
			new AnimatedInst(Pos(halfw, 100), get_sprite_by_name("logo")));
	gs->add_instance(
			new AnimatedInst(Pos(halfw - 100, 500), -1, -1, Posf(), Posf(),
					AnimatedInst::DEPTH, HELP_TEXT, Colour(255, 255, 255)));

	setup_mainmenu_buttons(gs, &exit, halfw, 300);

	for (; gs->update_iostate() && !gs->key_down_state(SDLK_RETURN) && !exit;) {
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;

	class_menu_loop(gs, width, height);
	gs->start_connection();

	if (gs->game_settings().conntype == GameSettings::SERVER) {
		connection_menu_loop(gs, width, height);
	}
}

static void choose_fighter(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().classtype = get_class_by_name("Fighter");
}
static void choose_mage(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().classtype = get_class_by_name("Mage");
}
static void choose_druid(GameState* gs, GameInst* _, void* flag) {
//	*(bool*) flag = true;
//	gs->game_settings().classtype = get_class_by_name("Druid");
}
static void choose_archer(GameState* gs, GameInst* _, void* flag) {
	*(bool*) flag = true;
	gs->game_settings().classtype = get_class_by_name("Archer");
}

static void setup_classmenu_buttons(GameState* gs, bool* exit, int x, int y) {
	ObjCallback chfighter(choose_fighter, exit);
	ObjCallback chmage(choose_mage, exit);
	ObjCallback chdruid(choose_druid, exit);
	ObjCallback charcher(choose_archer, exit);

	x -= 256 - 64;
	gs->add_instance(
			new ButtonInst("Mage", get_sprite_by_name("wizard_icon"), x, y,
					chmage, COL_GOLD));
	x += 128;
	gs->add_instance(
			new ButtonInst("Fighter", get_sprite_by_name("fighter_icon"), x, y,
					chfighter, COL_GOLD));
	x += 128;
	gs->add_instance(
			new ButtonInst("Archer", get_sprite_by_name("archer_icon"), x, y,
					charcher, COL_GOLD));
	x += 128;
	gs->add_instance(
			new ButtonInst("Druid", get_sprite_by_name("druid_icon"), x, y,
					chdruid, COL_LIGHT_GRAY));
	x += 128;
}

void class_menu_loop(GameState* gs, int width, int height) {
	bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(-1, width, height));
	gs->view().x = 0;
	gs->view().y = 0;

	gs->add_instance(
			new AnimatedInst(Pos(halfw, 100), get_sprite_by_name("logo")));
	setup_classmenu_buttons(gs, &exit, halfw, 300);

	for (; gs->update_iostate() && !gs->key_down_state(SDLK_RETURN) && !exit;) {
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;
}

static void start_game_with_current_players(GameState* gs, GameInst* _,
		void* flag) {
	*(bool*) flag = true;
	gs->game_settings().conntype = GameSettings::SERVER;
}

static void draw_connectors(GameState* gs, GameInst* inst, void* flag) {
	static Colour colours[] = { COL_BABY_BLUE, COL_PALE_YELLOW,
			COL_PALE_RED, COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY };
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
void connection_menu_loop(GameState* gs, int width, int height) {
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

	for (; gs->update_iostate() && !gs->key_down_state(SDLK_RETURN) && !exit;) {
		gs->net_connection().poll_messages();
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;
}
