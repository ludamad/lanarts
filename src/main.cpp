
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
#include "libyaml/yaml.h"
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


void init_system(bool fullscreen, int w, int h){
	init_sdl_gl(fullscreen,w,h);
//	init_units_list();
//	init_tiles_list();
	init_game_data();
}
void random_location(GameState* gs, obj_id id){
	GameInst* inst = gs->get_instance(id);

}

int main(int argc, char** argv) {
	int world_width = 128*TILE_SIZE, world_height = 128*TILE_SIZE;
//	int windoww = 640, windowh = 480;
//  int vieww = 480, viewh = 480;
	int hudw = 160;
	int windoww = 960, windowh = 720;
	int vieww = windoww -hudw, viewh = windowh;
	char* host = argv[1], * port = argv[2];
	bool cont = true;
	SDL_Event event;
	//Initialize the game tiles and empty game state
	GameState* gs = new GameState(world_width,world_height, vieww, viewh);

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
		return 0;
	}
	
	init_system(false/*Not fullscreen*/, windoww, windowh);
	gs->reset_level();

//	gs->add_instance( new TestInst(0,0));
	for (int i = 0; cont; i++) {
		clock_t start = clock();

		if (gs->key_down_state(SDLK_F1)) {
			for (int repeat = 0; repeat < 4; repeat++){
				cont = gs->step();
			//	if (!cont) break;
			}
		}
		cont = gs->step();
		gs->draw();
		clock_t end = clock();
		int delayms = 14 - (end-start)*1000/CLOCKS_PER_SEC;
		if (delayms > 0)
			SDL_Delay(delayms);
	}
	
	SDL_Quit();
	return 0;
}
