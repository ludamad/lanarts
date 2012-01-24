/*
 * GameTiles.cpp
 *
 *  Created on: 2011-10-27
 *      Author: 100397561
 */
#include "GameState.h"
#include "GameTiles.h"
#include "../data/tile_data.h"
#include "../display/display.h"
#include <SDL_opengl.h>
#include <cstring>

#include "../procedural/roomgen.h"
#include "../fov/fov.h"
/*
void GameTiles::draw(GameState* gs) {
	GameView& view = gs->window_view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= width) max_tilex = width - 1;
	if (max_tiley >= height) max_tiley = height - 1;
	GameInst* player = gs->get_instance(gs->player_id());
	const int sub_sqrs = 2;

	fov f(gs, 7, player->x*sub_sqrs/TILE_SIZE, player->y*sub_sqrs/TILE_SIZE, sub_sqrs);

	char matches[sub_sqrs*sub_sqrs];
	for (int y = min_tiley; y <= max_tiley; y++){
		for (int x = min_tilex; x <= max_tilex; x++){
			int tile = tiles[y*width + x];
			f.matches(x,y, matches);
			GLImage* img = &game_tile_data[tile].img;
			bool has_match = false;
			for (int i = 0; i < sub_sqrs*sub_sqrs; i++){
				if (matches[i]) {
					seen_tiles[y*width+x] = 1;
					has_match = true;
					break;
				}
			}
			image_display(img, x*TILE_SIZE - view.x, y*TILE_SIZE - view.y);
		}
	}
}*/


void GameTiles::draw(GameState* gs) {
	GameView& view = gs->window_view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= width) max_tilex = width - 1;
	if (max_tiley >= height) max_tiley = height - 1;
	GameInst* player = gs->get_instance(gs->player_id());
	const int sub_sqrs = 2;

	fov f(gs, 7, player->x*sub_sqrs/TILE_SIZE, player->y*sub_sqrs/TILE_SIZE, sub_sqrs);

	char matches[sub_sqrs*sub_sqrs];
	for (int y = min_tiley; y <= max_tiley; y++){
		for (int x = min_tilex; x <= max_tilex; x++){
			int tile = tiles[y*width + x];
			f.matches(x,y, matches);
			GLImage* img = &game_tile_data[tile].img;
			bool has_match = false;
			for (int i = 0; i < sub_sqrs*sub_sqrs; i++){
				if (matches[i]) {
					seen_tiles[y*width+x] = 1;
					has_match = true;
					break;
				}
			}
			if (has_match && tile == 1)
				image_display(img, x*TILE_SIZE - view.x, y*TILE_SIZE - view.y);
			else
				image_display_parts(img, x*TILE_SIZE - view.x, y*TILE_SIZE - view.y, 2, matches);
		}
	}
}

void GameTiles::generate_level(){
	generate_random_level(rs);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int ind = y * width + x;
			//printf(sqr[ind].passable ? "-" : "X");
			tiles[ind] = 1 - rs.sqrs[ind].passable;
		}
		//printf("\n");
	}
	//for (int i = 0; i < 80; i++) printf("-");
}
GameTiles::GameTiles(int width, int height, bool gen_level) :
	width(width), height(height), rs(width, height) {
		seen_tiles = new char[width*height];
		tiles = new int[width * height];
		memset(tiles, 0, width * height * sizeof (int));
		memset(seen_tiles, 0, width * height);

		if (gen_level) generate_level();
}

