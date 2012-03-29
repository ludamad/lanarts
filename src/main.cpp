
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

int main(int argc, char** argv) {
	GameSettings settings;
	init_system(settings);

	int world_width = 128*TILE_SIZE, world_height = 128*TILE_SIZE;

	int windoww = settings.view_width, windowh = settings.view_height;
	int vieww = windoww -HUD_WIDTH, viewh = windowh;

	//Initialize the game state and start the level
	GameState* gs = new GameState(settings, world_width,world_height, vieww, viewh);
	gs->reset_level();

	SDL_Event event;
	bool paused = false, cont = true;
//	gs->add_instance( new TestInst(0,0));
	for (int i = 0; cont; i++) {
		clock_t start = clock();

		if (gs->key_press_state(SDLK_F2)) {
			init_game_data();
		}
		if (gs->key_press_state(SDLK_F3)) {
			gs->regen_level();
		}
		if (gs->key_press_state(SDLK_F4)) {
			paused = !paused;
		}
		if (!paused){
			if (gs->key_down_state(SDLK_F1)) {
				for (int repeat = 0; repeat < 4; repeat++){
					cont = gs->step();
				//	if (!cont) break;
				}
			}
			cont = gs->step();
		} else
			gs->update_iostate();
		gs->draw();
		clock_t end = clock();
		int delayms = 14 - (end-start)*1000/CLOCKS_PER_SEC;
		if (delayms > 0)
			SDL_Delay(delayms);
	}
	
	SDL_Quit();
	return 0;
}
