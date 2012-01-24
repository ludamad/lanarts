
#include <iostream>
#include <SDL.h>
#include "display/display.h"
#include "world/GameState.h"
#include <vector>
#include "data/game_data.h"
#include "world/objects/PlayerInst.h"
#include "world/objects/EnemyInst.h"

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
	//FT_Init_FreeType( &library );
	obj_id player, enemy;
	int world_width = 128*TILE_SIZE, world_height = 128*TILE_SIZE;
//	int windoww = 640, windowh = 480;
//	int vieww = 480, viewh = 480;
	int windoww = 1280, windowh = 960;
	int vieww = 960, viewh = 960;
	char* host = argv[1], * port = argv[2];
	bool cont = true;
	SDL_Event event;
	//Initialize the game tiles and empty game state
	GameState gs(world_width,world_height, vieww, viewh);

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
		return 0;
	}
	init_system(false, windoww, windowh);
	int px,py,ex,ey;
	RoomgenSettings& rs = gs.tile_grid().room_settings();
	random_location(rs, px, py);
	random_location(rs, ex, ey);
	player = gs.add_instance(
			new PlayerInst(px*32+16,py*32+16));
	enemy = gs.add_instance( new EnemyInst(ex*32+16,ey*32+16));
	gs.window_view().sharp_center_on(px*32+16,py*32+16);
	for (int i = 0; cont; i++) {
		cont = gs.step();
		gs.draw();

	}
	return 0;
}
