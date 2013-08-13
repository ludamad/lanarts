/*
 * Minimap.cpp:
 *  Handles drawing & state of a minimap
 */

#include "draw/TileEntry.h"
#include "draw/colour_constants.h"

#include "gamestate/GameState.h"

#include "objects/player/PlayerInst.h"
#include "objects/FeatureInst.h"

#include <lcommon/math_util.h>

#include <SDL_opengl.h>
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


inline static void set_colour(char* buff, int x, int y, int ptw, Colour col) {
	int loc = y * ptw + x;
	buff[loc * 4] = col.b;
	buff[loc * 4 + 1] = col.g;
	buff[loc * 4 + 2] = col.r;
	buff[loc * 4 + 3] = col.a;
}

static void draw_rect2d(char* buff, int w, int h, BBox bbox, const Colour& col) {
	for (int yy = bbox.y1; yy < bbox.y2; yy++)
		for (int xx = bbox.x1; xx < bbox.x2; xx++) {
			if (xx == bbox.x1 || yy == bbox.y1 || xx == bbox.x2 - 1 || yy == bbox.y2 - 1) {
				set_colour(buff, xx, yy, w, col);
			}
		}
}

static void world2minimapbuffer(GameState* gs, char* buff,
		const BBox& shown, int w, int h, int ptw, int pth) {
	GameTiles& tiles = gs->tiles();
	GameView& view = gs->view();

	bool minimap_reveal = gs->key_down_state(SDLK_z);

	int stairs_down = res::tileid("stairs_down");
	int stairs_up = res::tileid("stairs_up");
	for (int y = 0; y < shown.height(); y++) {
		char* iter = buff + y * ptw * 4;
		for (int x = 0; x < shown.width(); x++) {
			Pos xy(x + shown.x1, y + shown.y1);
			int tile = tiles.get(xy).tile;
			int seen = tiles.is_seen(xy) || minimap_reveal;
			if (seen) {
				if (tile == stairs_down || tile == stairs_up) {
					iter[0] = 255, iter[1] = 0, iter[2] = 0, iter[3] = 255;
				} else if (!tiles.is_solid(xy)) {/*floor*/
					iter[0] = 100, iter[1] = 100, iter[2] = 100, iter[3] = 255;
				} else { //if (tile == 1){/*wall*/
					iter[0] = 255, iter[1] = 255, iter[2] = 255, iter[3] = 255;
				}
			}
			iter += 4;
		}
	}
	std::vector<GameInst*> instances = gs->get_level()->game_inst_set().to_vector();

	for (int i = 0; i < instances.size(); i++) {
		GameInst* instance = instances[i];
		bool is_enemy = dynamic_cast<EnemyInst*>(instance);
		bool is_feature = dynamic_cast<FeatureInst*>(instance);
		if (is_enemy || is_feature) {
			int ex = instance->x / TILE_SIZE;
			int ey = instance->y / TILE_SIZE;
			if (!shown.contains(ex, ey)) {
				continue;
			}

			bool seen = tiles.is_seen(Pos(ex, ey));
			if (is_enemy) {
				seen = seen && gs->object_visible_test(instance);
			} else if (is_feature) {
				bool was_seen =
						dynamic_cast<FeatureInst*>(instance)->has_been_seen();
				seen = seen && was_seen;
			}
			if (!seen && !minimap_reveal) {
				continue;
			}

			if (is_enemy) {
				set_colour(buff, ex - shown.x1, ey - shown.y1, ptw, Colour(255, 0, 0));
			} else if (is_feature) {
				set_colour(buff, ex - shown.x1, ey - shown.y1, ptw, Colour(0, 255, 0));
			}
		}
	}
}

static const int MINIMAP_DIM_MAX = 128;
static const Size MINIMAP_SIZE_MAX(MINIMAP_DIM_MAX, MINIMAP_DIM_MAX);

Minimap::Minimap(const BBox& minimap_max_bounds) :
		minimap_max_bounds(minimap_max_bounds), minimap_arr(NULL) {
	scale = 2;
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

static BBox find_portion(Pos center, Size desired_size, Size world_size) {
	Pos try_xy = center - Pos(desired_size.w/2, desired_size.h/2);
	try_xy = Pos(std::min(try_xy.x, world_size.w - desired_size.w), std::min(try_xy.y, world_size.h - desired_size.h));
	BBox world_bbox(Pos(), world_size);
	BBox try_bbox = BBox(try_xy, desired_size).resized_within(world_bbox);
	return BBox(try_bbox.left_top(), desired_size).resized_within(world_bbox);
}

// Draws a minimap from the contents of gs->tile_grid()
void Minimap::draw(GameState* gs) {

	GameTiles& tiles = gs->tiles();
	GameView& view = gs->view();
	BBox bbox = minimap_bounds(gs);

	BBox view_region = view.tile_region_covered();
	// Portion of world displayed on minimap
	BBox world_portion = find_portion(view_region.center(), MINIMAP_SIZE_MAX, tiles.size());
	printf("WORLD PORTION (%d %d) to (%d %d) size = (%d %d)\n", world_portion.x1, world_portion.y1, world_portion.x2, world_portion.y2, world_portion.width(), world_portion.height());

	int ptw = power_of_two_round(MINIMAP_DIM_MAX), pth = power_of_two_round(
			MINIMAP_DIM_MAX);
	if (!minimap_arr) {
		minimap_arr = new char[ptw * pth * 4];
	}

	init_minimap_buff(minimap_arr, ptw, pth);

	world2minimapbuffer(gs, minimap_arr, world_portion, bbox.width(), bbox.height(), ptw, pth);

	BBox adjusted_view_region = view_region.translated(-world_portion.x1, -world_portion.y1);
	PlayerInst* player = gs->local_player();
	if (player) {
		int arr_x = (player->x / TILE_SIZE) - world_portion.x1, arr_y = (player->y / TILE_SIZE) - world_portion.y1;
//		fill_buff2d(minimap_arr, ptw, pth, arr_x - arr_x % 2, arr_y - arr_y % 2,
//				Colour(255, 180, 99), 2, 2);
		fill_buff2d(minimap_arr, ptw, pth, arr_x, arr_y, Colour(255, 180, 99), 1, 1);
		draw_rect2d(minimap_arr, ptw, pth, adjusted_view_region, Colour(255, 180, 99));
	}
	minimap_buff.from_bytes(Size(ptw, pth), minimap_arr);

	Size size(MINIMAP_DIM_MAX / scale, MINIMAP_DIM_MAX / scale);
	BBox draw_region(adjusted_view_region.center() - Pos(size.w / 2, size.h / 2), size);
	draw_region = find_portion(adjusted_view_region.center(), size, world_portion.size());

	// Prevent blurriness when scaling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	minimap_buff.draw(ldraw::DrawOptions(draw_region).scale(SizeF(scale,scale)), bbox.left_top() + Pos(25,25));
	if (bbox.contains(gs->mouse_pos())) {
		if (gs->mouse_left_click()) {
			scale *= 2;
			if (scale > 8) {
				scale = 1;
			}
		} else if (gs->mouse_right_click()) {
			scale /= 2;
			if (scale < 1) {
				scale = 8;
			}
		}
	}
}
