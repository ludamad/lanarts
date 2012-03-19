/*
 * GameHud.cpp
 *
 *  Created on: 2011-11-06
 *      Author: 100400530
 */

#include "GameHud.h"
#include "../util/draw_util.h"
#include "GameState.h"
#include "objects/GameInst.h"
#include "GameInstSet.h"
#include "objects/PlayerInst.h"
#include "../data/tile_data.h"

static void draw_player_stats(PlayerInst* player, int x, int y) {
	Stats& s = player->stats();
	gl_draw_rectangle(x, y, 100, 10, Colour(255, 0, 0));
	gl_draw_rectangle(x, y, 100 * s.hp / s.max_hp, 10, Colour(0, 255, 0));
	gl_draw_rectangle(x, y + 15, 100, 10, Colour(200, 200, 200));
	gl_draw_rectangle(x, y + 15, 100 * s.mp / s.max_mp, 10, Colour(0, 0, 255));
}
static void fill_buff2d(char* buff, int w, int h, int x, int y,
		const Colour& col, int rw = 2, int rh = 2) {
	for (int yy = y; yy < y + rh; yy++)
		for (int xx = x; xx < x + rw; xx++) {
			if (yy > 0 && yy < h && xx > 0 && xx < h) {
				int loc = yy * w + xx;
				buff[loc * 4] = col.b;
				buff[loc * 4 + 1] = col.g;
				buff[loc * 4 + 2] = col.r;
				buff[loc * 4 + 3] = col.a;
			}
		}
}

static void draw_rect2d(char* buff, int w, int h, int x, int y, int mx, int my,
		const Colour& col) {
	for (int yy = y; yy < my; yy++)
		for (int xx = x; xx < mx; xx++) {
			if (xx == x || yy == y || xx == mx - 1 || yy == my - 1) {
				int loc = yy * w + xx;
				buff[loc * 4] = col.b;
				buff[loc * 4 + 1] = col.g;
				buff[loc * 4 + 2] = col.r;
				buff[loc * 4 + 3] = col.a;
			}
		}
}

void GameHud::draw_minimap(GameState* gs, int subx, int suby) {
	//Draw a mini version of the contents of gs->tile_grid()
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();

	bool pressed_z = gs->key_press_state(SDLK_z);
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	int tilew = tiles.tile_width(), tileh = tiles.tile_height();
	int ptw = power_of_two(tilew), pth = power_of_two(tileh);
	if (!minimap_arr) {
		minimap_arr = new char[ptw * pth * 4];
	}
	for (int i = 0; i < ptw * pth; i++) {
		minimap_arr[i * 4] = 0;
		minimap_arr[i * 4 + 1] = 0;
		minimap_arr[i * 4 + 2] = 0;
		minimap_arr[i * 4 + 3] = 255;
	}

	for (int y = 0; y < tileh; y++) {
		char* iter = minimap_arr + y * ptw * 4;
		for (int x = 0; x < tilew; x++) {
			int tile = tiles.get(x, y);
			int seen = tiles.seen(x, y) || pressed_z;
			if (seen) {
				if (tile > TILE_STONE_WALL) {/*floor*/
					iter[0] = 255, iter[1] = 255, iter[2] = 255, iter[3] = 255;
				} else { //if (tile == 1){/*wall*/
					iter[0] = 100, iter[1] = 100, iter[2] = 100, iter[3] = 255;
				}
			}
			iter += 4;
		}
	}
	GameInst* inst = gs->get_instance(gs->player_id());
	int arr_x = (inst->x / TILE_SIZE), arr_y = (inst->y / TILE_SIZE);
	fill_buff2d(minimap_arr, tilew, tileh, arr_x - arr_x % 2, arr_y - arr_y % 2,
			Colour(255, 180, 99), 2, 2);
	draw_rect2d(minimap_arr, tilew, tileh, min_tilex, min_tiley, max_tilex,
			max_tiley, Colour(255, 180, 99));//
	gl_image_from_bytes(&minimap_buff, ptw, pth, minimap_arr);
	image_display(&minimap_buff, subx, suby);
	//gl_printf(gs->primary_font(),Colour(255,255,255),0,100,"Whatup");
}
void GameHud::draw(GameState* gs) {
	gl_set_drawing_area(x, y, _width, _height);

	GameInst* player_inst = gs->get_instance(gs->player_id());
	gl_draw_rectangle(0, 0, _width, _height, bg_colour);
	if (player_inst)
		draw_player_stats((PlayerInst*) player_inst, 32, 32);
	else {
		//player = state->get_instance(0);
	}
	draw_minimap(gs, 20, 64);
	gl_printf(gs->primary_font(), Colour(255,255,255),_width/2-15,10,"Lanarts");
}

GameHud::GameHud(int x, int y, int width, int height) :
		x(x), y(y), _width(width), _height(height), bg_colour(0, 0, 0), minimap_arr(
				NULL) {
}
GameHud::~GameHud() {
	delete[] minimap_arr;
}
