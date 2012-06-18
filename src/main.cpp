#include <iostream>
#include <vector>
#include <ctime>
#include <SDL.h>
#include <SDL_opengl.h>

#include "data/game_data.h"

#include "display/display.h"

#include "lua/lua_api.h"

#include "world/GameState.h"

#include "world/objects/PlayerInst.h"
#include "world/objects/EnemyInst.h"

#include "world/utility_objects/AnimatedInst.h"
#include "world/utility_objects/ButtonInst.h"

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

static void set_flag_callback(void* flag) {
	*(bool*)flag = true;
}

static const char HELP_TEXT[] = "Movement: WASD or Arrow Keys\n"
		"Use Attack or Attack Spell: j or left-click\n"
		"Switch Auto-Target (for use with 'j'): k\n"
		"Switch Attack Spell: space\n"
		"Switch to Melee: m or e\n"
		"Use Blink: h or right-click\n"
		"Use Item: click item or keys 1 through 9\n"
		"Use Stairs: < and > or mouse wheel\n"
		"Regenerate: Hold 'r'\n";

static void menu_loop(GameState* gs, int width, int height) {
	bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->window_view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(0, 0, 0, width, height));
	gs->get_level()->level_number = -1;
	gs->window_view().x = 0;
	gs->window_view().y = 0;

	gs->add_instance(new AnimatedInst(halfw, 100, get_sprite_by_name("logo")));
	gs->add_instance(
			new AnimatedInst(halfw - 100, 500, -1, -1, 0.0f, 0.0f, AnimatedInst::DEPTH, HELP_TEXT,
					Colour(255, 255, 255)));
	gs->add_instance(
			new ButtonInst("START", BBox(halfw - 60, 400, halfw + 60, 430),
					set_flag_callback, &exit));

	for (; gs->update_iostate() && !gs->key_down_state(SDLK_RETURN) && !exit;) {
		gs->get_level()->inst_set.step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->window_view() = prevview;
}

static void game_loop(GameState* gs) {

	bool paused = false, cont = true;

	unsigned long draw_time = 5 * CLOCKS_PER_SEC / 1000;

	unsigned long draw_events = 1;

	unsigned long step_time = 0;
	unsigned long step_events = 1;

	GameSettings& settings = gs->game_settings();

	const clock_t per_frame = settings.time_per_step * CLOCKS_PER_SEC / 1000;

	gs->pre_step();
	for (int i = 1; cont; i++) {

		if (gs->key_press_state(SDLK_F2)) {
			init_game_data(gs->get_luastate());
			init_lua_data(gs, gs->get_luastate());
		}
		if (gs->key_press_state(SDLK_F3)) {
			gs->game_world().regen_level(gs->get_level()->roomid);
		}
		if (gs->key_press_state(SDLK_F4)) {
			paused = !paused;
		}

		//Draw event

		clock_t stepndraw_start = clock(), stepndraw_end;
		bool draw_this_step = (i > 1 && i % settings.steps_per_draw == 0);
		if (draw_this_step)
			gs->draw();

		clock_t draw_end = clock();
		draw_events++;
		draw_time += draw_end - stepndraw_start;

		int repeat_amount = 1;
		if (gs->key_down_state(SDLK_F1))
			repeat_amount = 4;

		for (int repeat = 0; repeat < repeat_amount; repeat++) {
			//Step event

			clock_t step_start = clock();
			if (!paused) {
				gs->step();
			}
			stepndraw_end = clock();
			step_events++;
			int len = stepndraw_end - step_start;
			step_time += len;

			bool is_done = (!paused && !gs->pre_step())
					|| (paused && !gs->update_iostate());
			if (is_done) {
				cont = false;
				break;
			}
		}

		clock_t thisdraw = draw_end - stepndraw_start, thisstep = stepndraw_end
				- draw_end;
		clock_t time_to_wait = per_frame * settings.steps_per_draw
				- (thisdraw / settings.steps_per_draw
						+ thisstep * settings.steps_per_draw);
		if (draw_this_step && time_to_wait > 0) {
			int delayms = time_to_wait * 1000 / CLOCKS_PER_SEC;
			if (delayms > 0)
				SDL_Delay(delayms);
		}
	}

	printf("Step time: %f\n",
			float(step_time) * 1000 / CLOCKS_PER_SEC / step_events);
	printf("Draw time: %f\n",
			float(draw_time) * 1000 / CLOCKS_PER_SEC / draw_events);
}

int main(int argc, char** argv) {
	lua_State* L = lua_open();

	GameSettings settings;
	init_system(settings, L);

	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww - HUD_WIDTH, viewh = windowh;

	//Initialize the game state and start the level
	//GameState claims ownership of the passed lua_State*

	GameState* gs = new GameState(settings, L, vieww, viewh);

	gs->update_iostate(); //for first iteration

	menu_loop(gs, windoww, windowh);

	gs->init_game();

	game_loop(gs);

	SDL_Quit();

	return 0;
}
