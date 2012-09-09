#include <iostream>
#include <vector>
#include <ctime>

#include <SDL.h>
#include <SDL_opengl.h>

#include <net/lanarts_net.h>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}
#include "data/game_data.h"

#include "display/display.h"

#include "gamestate/GameState.h"

#include "interface/ButtonInst.h"

#include "lua/lua_api.h"
#include "objects/enemy/EnemyInst.h"

#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"

#include "tests/tests.h"

#include "util/Timer.h"

using namespace std;

#ifdef __WIN32
#define main SDL_main
#endif

void init_system(GameSettings& settings, lua_State* L) {
	settings = load_settings_data("settings.yaml");
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(0);
	}
	lanarts_net_init(true);
	init_sdl_gl(settings.fullscreen, settings.view_width, settings.view_height);
	settings = init_game_data(L);
}

void save_game_to_file(GameState* gs, const char* filename) {
	FILE* file = fopen(filename, "wb");
	SerializeBuffer sb = SerializeBuffer::file_writer(file);
	gs->serialize(sb);
	sb.flush();
	fclose(file);
}
void load_game_from_file(GameState* gs, const char* filename) {
	FILE* file = fopen(filename, "rb");
	SerializeBuffer sb = SerializeBuffer::file_reader(file);
	gs->deserialize(sb);
	fclose(file);
}

void main_menu_loop(GameState* gs, int width, int height);

const int HUD_WIDTH = 160;

static void game_loop(GameState* gs) {

	bool paused = false, cont = true;

	unsigned long draw_time = 0;

	unsigned long draw_events = 1;

	unsigned long step_time = 0;
	unsigned long accumulated_time = 0;

	unsigned long step_events = 1;

	GameSettings& settings = gs->game_settings();

	Timer total_timer, step_timer, draw_timer;

	gs->pre_step();
	for (int i = 1; cont; i++) {
		perf_timer_begin("**Game Frame**");
		total_timer.start();

		if (gs->key_press_state(SDLK_F2)) {
			if (gs->player_data().all_players().size() == 1) {
				init_game_data(gs->get_luastate());
				init_lua_data(gs, gs->get_luastate());
			}
		}
		if (gs->key_press_state(SDLK_F3)) {
			if (gs->player_data().all_players().size() == 1) {
				gs->game_world().regen_level(gs->get_level()->id());
			}
		}
		if (gs->key_press_state(SDLK_F4)) {
			paused = !paused;
		}
		gs->net_connection().consume_sync_messages(gs);
		if (gs->key_press_state(SDLK_F6)) {
			if (gs->player_data().all_players().size() == 1) {
				load_game_from_file(gs, "savefile.save");
			}
		}

//Draw event

		draw_timer.start();
		bool draw_this_step = (i > 1 && i % settings.steps_per_draw == 0);
		if (draw_this_step) {
			gs->draw();
		}

		draw_events++;
		draw_time += draw_timer.get_microseconds();

		int repeat_amount = 1;
		if (gs->key_down_state(SDLK_F1))
			repeat_amount = 4;

		for (int repeat = 0; repeat < repeat_amount; repeat++) {
			//Step event

			step_timer.start();
			if (!paused) {
				gs->step();
			}
			step_events++;
			step_time += step_timer.get_microseconds();

			// Do not move this. It is done after the step event,
			// but before actions are queued for the next turn
			if (gs->key_press_state(SDLK_F5)) {
				net_send_state_and_sync(gs->net_connection(), gs);
				save_game_to_file(gs, "savefile.save");
			}

			// The following will queue actions for the next step
			bool is_done = (!paused && !gs->pre_step())
					|| (paused && !gs->update_iostate());
			if (is_done) {
				cont = false;
				break;
			}
		}

		accumulated_time += total_timer.get_microseconds();

		long microwait = settings.time_per_step * 1000 * settings.steps_per_draw
				- accumulated_time;
		if (draw_this_step) {
			long delayms = microwait / 1000;
			if (delayms > 0) {
				SDL_Delay(delayms);
			}
			accumulated_time = 0;
		}
		perf_timer_end("**Game Frame**");
	}

	perf_print_results();

	printf("Step time: %f\n", float(step_time) / step_events / 1000);
	printf("Draw time: %f\n", float(draw_time) / draw_events / 1000);
}

int main(int argc, char** argv) {
//	run_unit_tests();

	lua_State* L = lua_open();

	GameSettings settings;
	init_system(settings, L);

	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww - HUD_WIDTH, viewh = windowh;

	//Initialize the game state and start the level
	//GameState claims ownership of the passed lua_State*

	GameState* gs = new GameState(settings, L, vieww, viewh);

	gs->update_iostate(); //for first iteration

	main_menu_loop(gs, windoww, windowh);

	gs->start_game();

	game_loop(gs);

	SDL_Quit();

	return 0;
}
