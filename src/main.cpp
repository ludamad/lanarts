#include <iostream>
#include <vector>
#include <ctime>
#include <SDL.h>
#include <SDL_opengl.h>
#include "display/display.h"
#include "data/game_data.h"
#include "world/GameState.h"
#include "world/objects/PlayerInst.h"
#include "world/objects/EnemyInst.h"
#include "world/objects/TestInst.h"

using namespace std;

#ifdef __WIN32
#define main SDL_main
#endif
/*
 void init_system(bool fullscreen, int w, int h){
 init_sdl_gl(fullscreen,w,h);
 init_window(w,h);
 }
 void world_display(TileSet& ts, const ViewPoint& vp){
 glClearColor( 0.0, 0.0, 0.0, 1.0 );
 glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 ts.draw(vp);
 update_display();
 glFinish();
 }*/

void init_system(GameSettings& settings) {
	settings = load_settings_data("res/settings.yaml");
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(0);
	}
	init_sdl_gl(settings.fullscreen, settings.view_width, settings.view_height);
	init_game_data();
	settings = load_settings_data("res/settings.yaml");
}

const int HUD_WIDTH = 160;

static void game_loop(GameState* gs) {

	bool paused = false, cont = true;

	unsigned long draw_time = 5 * CLOCKS_PER_SEC / 1000;
	unsigned long draw_events = 1;

	unsigned long step_time = 0;
	unsigned long step_events = 1;

	const clock_t per_frame = 12 * CLOCKS_PER_SEC / 1000;
	clock_t time_allowance = 0;

	gs->pre_step();
	for (int i = 0; cont; i++) {

		if (gs->key_press_state(SDLK_F2)) {
			init_game_data();
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
		gs->draw();
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

		clock_t time_to_wait = per_frame - (clock() - stepndraw_start);

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

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "world/lua/lua_api.h"

int main(int argc, char** argv) {
	GameSettings settings;
	init_system(settings);

	int world_width = 128 * TILE_SIZE, world_height = 128 * TILE_SIZE;

	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww - HUD_WIDTH, viewh = windowh;

	//Initialize the game state and start the level
	GameState* gs = new GameState(settings, world_width, world_height, vieww,
			viewh);
	gs->update_iostate(); //for first iteration
//
	//
	game_loop(gs);

	SDL_Quit();

	return 0;
}
