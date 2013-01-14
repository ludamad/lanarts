#include <cstring>
#include <SDL_opengl.h>

#include <ldraw/draw.h>

#include "draw/draw_sprite.h"

#include "draw/TileEntry.h"

#include "levelgen/levelgen.h"

#include "objects/player/PlayerInst.h"

#include <lcommon/SerializeBuffer.h>

#include "util/math_util.h"
#include "GameState.h"

#include "GameTiles.h"

GameTiles::GameTiles(int width, int height) :
		width(width), height(height), tiles(width * height) {
}

GameTiles::~GameTiles() {
}

int GameTiles::tile_width() {
	return width;
}

int GameTiles::tile_height() {
	return height;
}

Tile& GameTiles::get(int x, int y) {
	return tiles[y * width + x].tile;
}

bool GameTiles::is_seen(int x, int y) {
	return tiles[y * width + x].seen;
}

void GameTiles::set_seethrough(int x, int y, bool seethrough) {
	tiles[y * width + x].seethrough = seethrough;
}

void GameTiles::set_solid(int x, int y, bool solid) {
	tiles[y * width + x].solid = solid;
}

bool GameTiles::is_solid(int x, int y) {
	return tiles[y * width + x].solid;
}

bool GameTiles::is_seethrough(int x, int y) {
	return tiles[y * width + x].seethrough;
}

void GameTiles::clear() {
	memset(&tiles[0], 0, sizeof(TileState) * width * height);
}

void GameTiles::mark_all_seen() {
	for (int i = 0; i < tiles.size(); i++) {
		tiles[i].seen = true;
	}
}

void GameTiles::copy_to(GameTiles & t) const {
	t.width = width, t.height = height;
	t.tiles = tiles;
}

void GameTiles::pre_draw(GameState* gs) {
	perf_timer_begin(FUNCNAME);
	GameView& view = gs->view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= width)
		max_tilex = width - 1;
	if (max_tiley >= height)
		max_tiley = height - 1;
//	bool reveal_enabled = gs->key_down_state(SDLK_BACKQUOTE);
	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			Tile& tile = get(x, y);
			const ldraw::Image& img = res::tile(tile.tile).img(tile.subtile);
			if (/*reveal_enabled ||*/is_seen(x, y)) {
				img.draw(on_screen(gs, Pos(x * TILE_SIZE, y * TILE_SIZE)));
			}
		}
	}

	perf_timer_end(FUNCNAME);
}

void GameTiles::step(GameState* gs) {
	const int sub_sqrs = VISION_SUBSQRS;

	char matches[sub_sqrs * sub_sqrs];

	std::vector<PlayerInst*> players = gs->players_in_level();

	for (int i = 0; i < players.size(); i++) {
		PlayerInst* player = players[i];
		fov& f = player->field_of_view();
		BBox fovbox = f.tiles_covered();
		for (int y = std::max(fovbox.y1, 0);
				y <= std::min(fovbox.y2, height - 1); y++) {
			for (int x = std::max(fovbox.x1, 0);
					x <= std::min(fovbox.x2, width - 1); x++) {
				f.matches(x, y, matches);
				for (int i = 0; i < sub_sqrs * sub_sqrs; i++) {
					if (matches[i])
						tiles[y * width + x].seen = 1;
				}
			}
		}
	}
}
void GameTiles::post_draw(GameState* gs) {

	GameView& view = gs->view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= width)
		max_tilex = width - 1;
	if (max_tiley >= height)
		max_tiley = height - 1;
	const int sub_sqrs = VISION_SUBSQRS;

	if (/*gs->key_down_state(SDLK_BACKQUOTE) ||*/!gs->level_has_player()) {
		return;
	}
	perf_timer_begin(FUNCNAME);

	fov& mainfov = gs->local_player()->field_of_view();
	char matches[sub_sqrs * sub_sqrs];
	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			bool has_match = false, has_free = false;
			bool is_other_match = false;
			Tile& tile = get(x, y);

			std::vector<PlayerInst*> players = gs->players_in_level();

			for (int i = 0; i < players.size(); i++) {
				fov& f = players[i]->field_of_view();
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
				BBox tilebox(
						Pos(x * TILE_SIZE - view.x, y * TILE_SIZE - view.y),
						res::tile(tile.tile).size());
				if (!is_other_match) {
					if (!is_seen(x, y)) {
						ldraw::draw_rectangle(Colour(0, 0, 0), tilebox);
					} else {
						// Previously seen
						ldraw::draw_rectangle(Colour(0, 0, 0, 180), tilebox);
					}
				} else {
					ldraw::draw_rectangle(Colour(0, 0, 0, 60), tilebox);
				}
			}
		}
	}
	perf_timer_end(FUNCNAME);
}

void GameTiles::serialize(SerializeBuffer& serializer) {
	serializer.write(width);
	serializer.write(height);
	serializer.write_container(tiles);
}

static bool circle_line_test(int px, int py, int qx, int qy, int cx, int cy,
		float radsqr) {
	int dx, dy, t, rt, ddist;
	dx = qx - px;
	dy = qy - py;
	ddist = dx * dx + dy * dy;
	t = -((px - cx) * dx + (py - cy) * dy);
	//;/ ddist;

	/* Restrict t to within the limits of the line segment */
	if (t < 0)
		t = 0;
	else if (t > ddist)
		t = ddist;

	dx = (px + t * (qx - px) / ddist) - cx;
	dy = (py + t * (qy - py) / ddist) - cy;
	rt = (dx * dx) + (dy * dy);
	return rt < (radsqr);
}

bool GameTiles::radius_test(int x, int y, int rad, bool issolid, int ttype,
		Pos* hitloc) {
	int distsqr = (TILE_SIZE / 2 + rad), radsqr = rad * rad;
	distsqr *= distsqr; //sqr it

	int mingrid_x = (x - rad) / TILE_SIZE, mingrid_y = (y - rad) / TILE_SIZE;
	int maxgrid_x = (x + rad) / TILE_SIZE, maxgrid_y = (y + rad) / TILE_SIZE;
	int minx = squish(mingrid_x, 0, width), miny = squish(mingrid_y, 0, height);
	int maxx = squish(maxgrid_x, 0, width), maxy = squish(maxgrid_y, 0, height);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			TileState& tilestate = tiles[yy * width + xx];
			Tile& tile = tilestate.tile;

			bool istype = (tile.tile == ttype || ttype == -1);
			bool solidmatch = (tilestate.solid == issolid);
			if (solidmatch && istype) {
				int offset = TILE_SIZE / 2; //To and from center
				int cx = int(xx * TILE_SIZE) + offset;
				int cy = int(yy * TILE_SIZE) + offset;
				int ydist = cy - y;
				int xdist = cx - x;
				double ddist = ydist * ydist + xdist * xdist;
				if (ddist < distsqr
						|| circle_line_test(cx - offset, cy - offset,
								cx + offset, cy - offset, x, y, radsqr)
						|| circle_line_test(cx - offset, cy - offset,
								cx - offset, cy + offset, x, y, radsqr)
						|| circle_line_test(cx - offset, cy + offset,
								cx + offset, cy + offset, x, y, radsqr)
						|| circle_line_test(cx + offset, cy - offset,
								cx + offset, cy + offset, x, y, radsqr)) {
					if (hitloc)
						*hitloc = Pos(xx, yy);
					return true;
				}
			}
		}
		//printf("\n");
	}
	return false;
}

void GameTiles::deserialize(SerializeBuffer& serializer) {
	serializer.read(width);
	serializer.read(height);
//	for (int i = 0; i < tiles.size(); i++) {
//		tiles[i] = TileState();
//	}
	serializer.read_container(tiles);
}

