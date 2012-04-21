
#include <iostream>
#include <ctime>
#include <SDL.h>
#include <SDL_opengl.h>
#include "display/display.h"
#include "world/GameState.h"
#include <vector>
#include "data/game_data.h"
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


void init_system(GameSettings& settings){
	settings = load_settings_data("res/settings.yaml");
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
		exit(0);
	}
	init_sdl_gl(settings.fullscreen, settings.view_width, settings.view_height);
	init_game_data();
	settings = load_settings_data("res/settings.yaml");
}

const int HUD_WIDTH = 160;

static void game_loop(GameState* gs){

	bool paused = false, cont = true;

	unsigned long draw_time = 5*CLOCKS_PER_SEC/1000;
	unsigned long draw_events = 1;

	unsigned long step_time = 0;
	unsigned long step_events = 1;

	const clock_t per_frame = 14*CLOCKS_PER_SEC/1000;
	clock_t time_allowance = 0;

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
		if (!paused){
			if (gs->key_down_state(SDLK_F1)) {
				for (int repeat = 0; repeat < 4; repeat++){
					cont = gs->step();
					if (!cont) break;
				}
			}
			{
				//Draw event
				clock_t start = clock();
				cont = gs->step();
				clock_t end = clock();
				step_events++;
				int len = end-start;
				step_time += len;

				time_allowance = per_frame - len;
			}
		} else
			gs->update_iostate();
	//	if (time_allowance > draw_events/draw_time){
			//Draw event
			clock_t start = clock();
			gs->draw();
			clock_t end = clock();
			draw_events++;
			draw_time += end-start;
			time_allowance -= end-start;
		//}
	//	if (time_allowance > draw_events/draw_time){
			int delayms = time_allowance*1000/CLOCKS_PER_SEC;
			if (delayms > 0)
				SDL_Delay(delayms);
		//	time_allowance = 0;
	///	}
	}


	printf("Step time: %f\n", float(step_time)/step_events);
	printf("Draw time: %f\n", float(draw_time)/draw_events);
}

int main(int argc, char** argv) {
	GameSettings settings;
	init_system(settings);

	int world_width = 128*TILE_SIZE, world_height = 128*TILE_SIZE;

	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww -HUD_WIDTH, viewh = windowh;

	//Initialize the game state and start the level
	GameState* gs = new GameState(settings, world_width,world_height, vieww, viewh);
	gs->update_iostate();//for first iteration

	game_loop(gs);

	SDL_Quit();
	return 0;
}
