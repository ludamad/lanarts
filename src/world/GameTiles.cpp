/*
 * GameTiles.cpp
 *
 *  Created on: 2011-10-27
 *      Author: 100397561
 */
#include <cstring>
#include <SDL_opengl.h>

#include "GameState.h"
#include "GameTiles.h"

#include "../data/tile_data.h"
#include "../display/display.h"
#include "../util/draw_util.h"
#include "../procedural/levelgen.h"

void GameTiles::pre_draw(GameState* gs) {
	GameView& view = gs->window_view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= width)
		max_tilex = width - 1;
	if (max_tiley >= height)
		max_tiley = height - 1;

	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			int tile = tiles[y * width + x];
			GLImage* img = &game_tile_data[tile].img;
			image_display(img, x * TILE_SIZE - view.x, y * TILE_SIZE - view.y);
		}
	}

}

void GameTiles::post_draw(GameState* gs) {
	GameView& view = gs->window_view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= width)
		max_tilex = width - 1;
	if (max_tiley >= height)
		max_tiley = height - 1;
	const int sub_sqrs = VISION_SUBSQRS;
	if (gs->local_playerid() == 0) return;


	fov& mainfov = *gs->player_controller().local_playerfov();
	for (int i = 0; i <  gs->player_controller().player_fovs().size(); i++){
		fov& f = *gs->player_controller().player_fovs()[i];
		char matches[sub_sqrs * sub_sqrs];
		for (int y = min_tiley; y <= max_tiley; y++) {
			for (int x = min_tilex; x <= max_tilex; x++) {
				bool has_match = false, has_free = false;
				int tile = tiles[y * width + x];
				GLImage* img = &game_tile_data[tile].img;

				f.matches(x, y, matches);
				for (int i = 0; i < sub_sqrs * sub_sqrs; i++) {
					if (matches[i]) {
						seen_tiles[y * width + x] = 1;
						has_match = true;
					} else {
						has_free = true;
					}
				}

				if (&f != &mainfov) continue;

				//Do not draw black if we have a match, and we see a wall
				if (!has_match) {
					if (!seen_tiles[y * width + x]) {
						gl_draw_rectangle(x * TILE_SIZE - view.x,
								y * TILE_SIZE - view.y, img->width, img->height);
					} else {
						gl_draw_rectangle(x * TILE_SIZE - view.x,
								y * TILE_SIZE - view.y, img->width, img->height,
								Colour(0, 0, 0, 120));
					}
				}
	//			else if (has_match && has_free && tile != 1) {
	//				gl_draw_rectangle_parts(x * TILE_SIZE - view.x,
	//						y * TILE_SIZE - view.y, img->width, img->height, 2,
	//						matches);
	//			}
			}
		}
	}
}
/*
void GameTiles::generate_tiles(MTwist& mt, GeneratedLevel& level) {
	int lw = level.width(), lh = level.height();
	int start_x = (width-lw)/2;
	int start_y = (height-lh)/2;
	int end_x = start_x + lw, end_y = start_y + lh;

	memset(tiles, 0, width * height * sizeof(int));
	memset(seen_tiles, 0, width * height);

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			int ind = y*width+x;
			Sqr& s = level.at(x-start_x, y-start_y);
			if (s.passable) {
				tiles[ind] = TILE_FLOOR;
				if (s.roomID){
//					if (s.marking)
					if (mt.rand(150) == 0){
						tiles[ind] = TILE_STAIR_DOWN;
					}
// 					tiles[ind] = TILE_MESH_0+s.marking;
				} else if (s.feature == SMALL_CORRIDOR){
					tiles[ind] = TILE_CORRIDOR_FLOOR;
				}
			} else {
				tiles[ind] = TILE_WALL;
				if (s.feature == SMALL_CORRIDOR){
					if (mt.rand(4) == 0){
						tiles[ind] = TILE_STONE_WALL;
					}
				}
			}
		}
	}
}*/
GameTiles::GameTiles(int width, int height) :
		width(width), height(height) {
	seen_tiles = new char[width * height];
	tiles = new int[width * height];
	memset(tiles, 0, width * height * sizeof(int));
	memset(seen_tiles, 0, width * height);
}

