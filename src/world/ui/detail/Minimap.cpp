/*
 * Minimap.cpp:
 *  Handles drawing & state of a minimap
 */

#include "../../../data/tile_data.h"

#include "../../../util/math_util.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "Minimap.h"

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

static void world2minimapbuffer(GameState* gs, char* buff,
		const BBox& world_portion, int w, int h, int ptw, int pth) {
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();

	bool minimap_reveal = gs->key_down_state(SDLK_z)
			|| gs->key_down_state(SDLK_BACKQUOTE);

	int stairs_down = get_tile_by_name("stairs_down");
	int stairs_up = get_tile_by_name("stairs_up");
	for (int y = 0; y < h; y++) {
		char* iter = buff + y * ptw * 4;
		for (int x = 0; x < w; x++) {
			int tile = tiles.get(x, y).tile;
			int seen = tiles.is_seen(x, y) || minimap_reveal;
			if (seen) {
				if (tile == stairs_down || tile == stairs_up) {
					iter[0] = 255, iter[1] = 0, iter[2] = 0, iter[3] = 255;
				} else if (!tiles.is_solid(x, y)) {/*floor*/
					iter[0] = 255, iter[1] = 255, iter[2] = 255, iter[3] = 255;
				} else { //if (tile == 1){/*wall*/
					iter[0] = 100, iter[1] = 100, iter[2] = 100, iter[3] = 255;
				}
			}
			iter += 4;
		}
	}
	const std::vector<obj_id>& enemy_ids =
			gs->monster_controller().monster_ids();
	for (int i = 0; i < enemy_ids.size(); i++) {
		GameInst* enemy = gs->get_instance(enemy_ids[i]);
		if (enemy) {
			int ex = enemy->x / TILE_SIZE;
			int ey = enemy->y / TILE_SIZE;
			int loc = ey * ptw + ex;
			if (!minimap_reveal && !tiles.is_seen(ex, ey))
				continue;
			if (!minimap_reveal && !gs->object_visible_test(enemy))
				continue;

			buff[loc * 4] = 0;
			buff[loc * 4 + 1] = 0;
			buff[loc * 4 + 2] = 255;
			buff[loc * 4 + 3] = 255;
		}
	}
//
//	int min_tilex, min_tiley;
//	int max_tilex, max_tiley;
//
//	view.min_tile_within(min_tilex, min_tiley);
//	view.max_tile_within(max_tilex, max_tiley);
}

const int MINIMAP_SIZEMAX = 128;

Minimap::Minimap(const BBox& minimap_max_bounds) :
		minimap_max_bounds(minimap_max_bounds), minimap_arr(NULL) {
}

BBox Minimap::minimap_bounds(GameState* gs) {
	int level_w = gs->get_level()->tile_width();
	int level_h = gs->get_level()->tile_height();

	int max_w = minimap_max_bounds.width(), max_h = minimap_max_bounds.height();

	int draw_x = minimap_max_bounds.x1 + (max_w - level_w) / 2;
	int draw_y = minimap_max_bounds.y1 + (max_h - level_h) / 2;

	return BBox(draw_x, draw_y, draw_x + level_w, draw_y + level_h);
}

static void init_minimap_buff(char* minimap_arr, int ptw, int pth) {
	int buffsize = ptw * pth;
	for (int i = 0; i < buffsize; i++) {
		minimap_arr[i * 4] = 0;
		minimap_arr[i * 4 + 1] = 0;
		minimap_arr[i * 4 + 2] = 0;
		minimap_arr[i * 4 + 3] = 255;
	}
}

void Minimap::draw(GameState* gs, float scale) {
	//Draw a mini version of the contents of gs->tile_grid()
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();
	BBox bbox = minimap_bounds(gs);

	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	int ptw = power_of_two(MINIMAP_SIZEMAX), pth = power_of_two(
			MINIMAP_SIZEMAX);
	if (!minimap_arr) {
		minimap_arr = new char[ptw * pth * 4];
	}

	init_minimap_buff(minimap_arr, ptw, pth);

	world2minimapbuffer(gs, minimap_arr, BBox(), bbox.width(), bbox.height(),
			ptw, pth);

	GameInst* inst = gs->get_instance(gs->local_playerid());
	if (inst) {
		int arr_x = (inst->x / TILE_SIZE), arr_y = (inst->y / TILE_SIZE);
		fill_buff2d(minimap_arr, ptw, pth, arr_x - arr_x % 2, arr_y - arr_y % 2,
				Colour(255, 180, 99), 2, 2);
		draw_rect2d(minimap_arr, ptw, pth, min_tilex, min_tiley, max_tilex,
				max_tiley, Colour(255, 180, 99)); //
	}

	gl_image_from_bytes(minimap_buff, ptw, pth, minimap_arr);

	gl_draw_image(minimap_buff, bbox.x1, bbox.y1);
}

