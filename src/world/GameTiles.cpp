/*
 * GameTiles.cpp
 *
 *  Created on: 2011-10-27
 *      Author: 100397561
 */
#include <cstring>
#include <SDL_opengl.h>

#include "GameTiles.h"
#include "GameState.h"

#include "../data/tile_data.h"

#include "../display/display.h"

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
			if (seen_tiles[y * width + x])
			gl_draw_image(img, x * TILE_SIZE - view.x, y * TILE_SIZE - view.y);
		}
	}

}

void GameTiles::step(GameState* gs) {
	GameView& view = gs->window_view();
	const int sub_sqrs = VISION_SUBSQRS;

	char matches[sub_sqrs * sub_sqrs];

	for (int i = 0; i < gs->player_controller().player_fovs().size(); i++) {
		fov& f = *gs->player_controller().player_fovs()[i];
		BBox fovbox = f.tiles_covered();
		for (int y = std::max(fovbox.y1, 0); y <= std::max(fovbox.y2, height); y++) {
			for (int x = std::max(fovbox.x1, 0); x <= std::min(fovbox.x2, width); x++) {
				bool has_match = false, has_free = false;
				bool is_other_match = false;
				int tile = tiles[y * width + x];
				GLImage* img = &game_tile_data[tile].img;

				f.matches(x, y, matches);
				for (int i = 0; i < sub_sqrs * sub_sqrs; i++) {
					if (matches[i])
						seen_tiles[y * width + x] = 1;
				}
			}
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

	if (gs->player_controller().player_ids().empty()) return;

	fov& mainfov = *gs->player_controller().local_playerfov();
	char matches[sub_sqrs * sub_sqrs];
	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			bool has_match = false, has_free = false;
			bool is_other_match = false;
			int tile = tiles[y * width + x];
			GLImage* img = &game_tile_data[tile].img;

			for (int i = 0; i < gs->player_controller().player_fovs().size();
					i++) {
				fov& f = *gs->player_controller().player_fovs()[i];
				f.matches(x, y, matches);
				for (int i = 0; i < sub_sqrs * sub_sqrs; i++) {
					if (matches[i]) {
						if (&f == &mainfov)
							has_match = true;
						else
							is_other_match = true;
					} else {
						has_free = true;
					}
				}
			}

			//Do not draw black if we have a match, and we see a wall
			if (!has_match) {
				if (!is_other_match) {
					if (!seen_tiles[y * width + x]) {
						gl_draw_rectangle(x * TILE_SIZE - view.x,
								y * TILE_SIZE - view.y, img->width,
								img->height);
					} else {
						gl_draw_rectangle(x * TILE_SIZE - view.x,
								y * TILE_SIZE - view.y, img->width, img->height,
								Colour(0, 0, 0, 180));
					}
				} else {
					gl_draw_rectangle(x * TILE_SIZE - view.x,
							y * TILE_SIZE - view.y, img->width, img->height,
							Colour(0, 0, 0, 60));
				}
			}
		}
	}

}

GameTiles::GameTiles(int width, int height) :
		width(width), height(height) {
	seen_tiles = new char[width * height];
	tiles = new int[width * height];
	memset(tiles, 0, width * height * sizeof(int));
	memset(seen_tiles, 0, width * height);
}

