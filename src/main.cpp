#include <iostream>
#include <vector>
#include <ctime>
#include <SDL.h>
#include <SDL_opengl.h>
#include "display/display.h"
#include "data/game_data.h"
#include "world/GameState.h"
#include "world/objects/ButtonInst.h"
#include "world/objects/PlayerInst.h"
#include "world/objects/EnemyInst.h"
#include "world/objects/AnimatedInst.h"
#include "world/objects/TestInst.h"
#include "world/lua/lua_api.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

using namespace std;

#ifdef __WIN32
#define main SDL_main
#endif

void init_system(GameSettings& settings, lua_State* L) {
	settings = load_settings_data("res/settings.yaml");
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(0);
	}
	init_sdl_gl(settings.fullscreen, settings.view_width, settings.view_height);
	init_game_data(L);
	settings = load_settings_data("res/settings.yaml");
}

const int HUD_WIDTH = 160;

static void leave_menu(void* flag) {
	*(bool*) flag = true;
}

static const char HELP_TEXT[] =
		"Movement: WASD or Arrow Keys\n"
		"Use Attack or Attack Spell: j or left-click\n"
		"Switch Auto-Target (for use with 'j'): k\n"
		"Switch Attack Spell: space\n"
		"Switch to Melee: m or e\n"
		"Use Blink: h or right-click\n"
		"Use Item: click item or keys 1 through 9\n"
		"Use Stairs: < and > or mouse wheel\n";

static void menu_loop(GameState* gs, int width, int height) {
	bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->window_view();
	GameLevelState* oldlevel = gs->level();

	gs->level() = new GameLevelState(0, 0, 0, width, height);
	gs->level()->level_number = -1;
	gs->window_view().x = 0;
	gs->window_view().y = 0;

	gs->add_instance(new AnimatedInst(halfw, 100, get_sprite_by_name("logo")));
	gs->add_instance(new AnimatedInst(halfw - 100, 500, -1, -1,  0.0f,0.0f, HELP_TEXT, Colour(255,255,255)));
	gs->add_instance(
			new ButtonInst("START", BBox(halfw - 60, 400, halfw + 60, 430),
					leave_menu, &exit));

	for (; gs->update_iostate() && !gs->key_down_state(SDLK_RETURN) && !exit;) {
		gs->level()->inst_set.step(gs);
		gs->draw(false);
	}

	delete gs->level();

	gs->level() = oldlevel;
	gs->window_view() = prevview;
}

//#define GAME_DRAW_EVERY_N_FRAMES 3

static void game_loop(GameState* gs) {

	bool paused = false, cont = true;

	unsigned long draw_time = 5 * CLOCKS_PER_SEC / 1000;

	unsigned long draw_events = 1;

	unsigned long step_time = 0;
	unsigned long step_events = 1;

	const clock_t per_frame = 12 * CLOCKS_PER_SEC / 1000;
	clock_t time_allowance = 0;

	gs->pre_step();
	for (int i = 1; cont; i++) {

		if (gs->key_press_state(SDLK_F2)) {
			init_game_data(gs->get_luastate());
		}
		if (gs->key_press_state(SDLK_F3)) {
			gs->game_world().regen_level(gs->level()->roomid);
		}
		if (gs->key_press_state(SDLK_F4)) {
			paused = !paused;
		}
		if (gs->key_down_state(SDLK_x)) {
			GameView& view = gs->window_view();
			int nx = gs->mouse_x() + view.x, ny = gs->mouse_y() + view.y;
			view.center_on(nx, ny);
		}

		//Draw event

		clock_t stepndraw_start = clock(), stepndraw_end;
#ifdef GAME_DRAW_EVERY_N_FRAMES
		if (i%GAME_DRAW_EVERY_N_FRAMES == 0) gs->draw();
#else
		gs->draw();
#endif
		clock_t draw_end = clock();
		draw_events++;
		draw_time += draw_end - stepndraw_start;
		time_allowance -= draw_end - stepndraw_start;

		if (!paused) {
			int repeat_amount = 1;
			if (gs->key_down_state(SDLK_F1))
				repeat_amount = 4;

			for (int repeat = 0; repeat < repeat_amount; repeat++) {
				//Step event

				clock_t step_start = clock();
				gs->step();
				stepndraw_end = clock();
				step_events++;
				int len = stepndraw_end - step_start;
				step_time += len;

				time_allowance = per_frame - len;
				bool is_done = (!paused && !gs->pre_step())
						|| (paused && !gs->update_iostate());
				if (is_done) {
					cont = false;
					break;
				}
			}
		}

#ifdef GAME_DRAW_EVERY_N_FRAMES
		clock_t time_to_wait = per_frame - (draw_time/i/GAME_DRAW_EVERY_N_FRAMES + step_time/i);
#else
		clock_t time_to_wait = per_frame - (clock() - stepndraw_start);
#endif
		if (time_to_wait > 0) {
			int delayms = time_to_wait * 1000 / CLOCKS_PER_SEC;
			if (delayms > 0)
				SDL_Delay(delayms);
			time_allowance = 0;
		}
	}

	printf("Step time: %f\n", float(step_time) / step_events);
	printf("Draw time: %f\n", float(draw_time) / draw_events);
}

int main(int argc, char** argv) {
	lua_State* L = lua_open();
	GameSettings settings;
	init_system(settings, L);

	int world_width = 128 * TILE_SIZE, world_height = 128 * TILE_SIZE;

	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww - HUD_WIDTH, viewh = windowh;

	//Initialize the game state and start the level
	//GameState claims ownership of the passed lua_State*
	GameState* gs = new GameState(settings, L, world_width, world_height, vieww,
			viewh);

	gs->update_iostate(); //for first iteration

	menu_loop(gs, windoww, windowh);

	gs->init_game();

	game_loop(gs);

	SDL_Quit();

	return 0;
}
