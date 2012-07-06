#include <cstring>
#include <SDL_opengl.h>

#include "../data/tile_data.h"

#include "../display/display.h"

#include "../procedural/levelgen.h"

#include "GameTiles.h"
#include "GameState.h"

#include "objects/PlayerInst.h"

GameTiles::GameTiles(int width, int height) :
		width(width), height(height), solid_tiles(width * height, true) {
	seen_tiles = new char[width * height];
	tiles = new Tile[width * height];
	memset(seen_tiles, 0, width * height);
}

GameTiles::~GameTiles() {
	delete[] tiles;
	delete[] seen_tiles;
}

int GameTiles::tile_width() {
	return width;
}

int GameTiles::tile_height() {
	return height;
}

Tile& GameTiles::get(int x, int y) {
	return tiles[y * width + x];
}

Tile* GameTiles::tile_array() {
	return tiles;
}

bool GameTiles::is_seen(int x, int y) {
	return seen_tiles[y * width + x];
}

void GameTiles::set_solid(int x, int y, bool solid) {
	solid_tiles[y * width + x] = solid;
}

bool GameTiles::is_solid(int x, int y) {
	return solid_tiles[y * width + x];
}

void GameTiles::clear() {
	memset(seen_tiles, 0, width * height);
	memset(tiles, 0, sizeof(int) * width * height);
}

void GameTiles::copy_to(GameTiles & t) const {
	t.width = width, t.height = height;
	memcpy(t.seen_tiles, seen_tiles, width * height);
	memcpy(t.tiles, tiles, sizeof(int) * width * height);
	t.solid_tiles = solid_tiles;
}

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
	bool reveal_enabled = gs->key_down_state(SDLK_BACKQUOTE);
	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			Tile& tile = get(x, y);
			GLimage& img = game_tile_data[tile.tile].img(tile.subtile);
			if (reveal_enabled || seen_tiles[y * width + x])
				gl_draw_image(img, x * TILE_SIZE - view.x,
						y * TILE_SIZE - view.y);
		}
	}

}

void GameTiles::step(GameState* gs) {
	const int sub_sqrs = VISION_SUBSQRS;

	char matches[sub_sqrs * sub_sqrs];

	const std::vector<obj_id>& pids = gs->player_controller().player_ids();

	for (int i = 0; i < pids.size(); i++) {
		PlayerInst* player = (PlayerInst*)gs->get_instance(pids[i]);
		fov& f = player->field_of_view();
		BBox fovbox = f.tiles_covered();
		for (int y = std::max(fovbox.y1, 0);
				y <= std::min(fovbox.y2, height - 1); y++) {
			for (int x = std::max(fovbox.x1, 0);
					x <= std::min(fovbox.x2, width - 1); x++) {
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

	if (gs->key_down_state(SDLK_BACKQUOTE)
			|| gs->player_controller().player_ids().empty())
		return;

	fov& mainfov = gs->local_player()->field_of_view();
	char matches[sub_sqrs * sub_sqrs];
	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			bool has_match = false, has_free = false;
			bool is_other_match = false;
			Tile& tile = get(x, y);
			GLimage& img = game_tile_data[tile.tile].img(tile.subtile);

			const std::vector<obj_id>& pids =
					gs->player_controller().player_ids();

			for (int i = 0; i < pids.size(); i++) {
				PlayerInst* player = (PlayerInst*)gs->get_instance(pids[i]);
				fov& f = player->field_of_view();
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
								y * TILE_SIZE - view.y, img.width, img.height);
					} else {
						gl_draw_rectangle(x * TILE_SIZE - view.x,
								y * TILE_SIZE - view.y, img.width, img.height,
								Colour(0, 0, 0, 180));
					}
				} else {
					gl_draw_rectangle(x * TILE_SIZE - view.x,
							y * TILE_SIZE - view.y, img.width, img.height,
							Colour(0, 0, 0, 60));
				}
			}
		}
	}

}
