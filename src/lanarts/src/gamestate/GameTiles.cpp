/*
 * GameTiles.cpp:
 *   Handles drawing of the tiles of the game, drawing of the fog-of-war,
 *   storage of tile information for a level. Stores if a tile has already
 *   been seen.
 */

#include <cstring>
#include <SDL_opengl.h>

#include <ldraw/draw.h>

#include "draw/draw_sprite.h"

#include "draw/TileEntry.h"

#include "levelgen/levelgen.h"

#include "objects/player/PlayerInst.h"

#include <lcommon/SerializeBuffer.h>

#include <lcommon/math_util.h>
#include "GameState.h"

#include "GameTiles.h"

GameTiles::GameTiles(const Size& size) :
		_tiles(size) {

	_solidity = new Grid<bool>(size, true);
}

GameTiles::~GameTiles() {
}

int GameTiles::tile_width() {
	return _tiles.size().w;
}

int GameTiles::tile_height() {
	return _tiles.size().h;
}

Size GameTiles::size() const {
	return _tiles.size();
}

Tile& GameTiles::get(const Pos& xy) {
	return _tiles[xy].tile;
}

bool GameTiles::is_seen(const Pos& xy) {
	return _tiles[xy].seen;
}

void GameTiles::set_seethrough(const Pos& xy, bool seethrough) {
	_tiles[xy].seethrough = seethrough;
}

void GameTiles::set_solid(const Pos& xy, bool solid) {
	(*_solidity)[xy] = solid;
}

bool GameTiles::is_solid(const Pos& xy) {
	return (*_solidity)[xy];
}

bool GameTiles::is_seethrough(const Pos& xy) {
	return _tiles[xy].seethrough;
}

void GameTiles::clear() {
	memset(_tiles.begin(), 0, sizeof(TileState) * size().area());
	_solidity->fill(false);
}

void GameTiles::mark_all_seen() {
	TileState* end = _tiles.end();
	for (TileState* iter = _tiles.begin(); iter != end; iter++) {
		iter->seen = true;
	}
}

void GameTiles::copy_to(GameTiles & t) const {
	t._solidity = _solidity;
	t._tiles = _tiles;
}

void GameTiles::pre_draw(GameState* gs) {
	perf_timer_begin(FUNCNAME);

	Size size = this->size();
	GameView& view = gs->view();

	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= size.w)
		max_tilex = size.w - 1;
	if (max_tiley >= size.h)
		max_tiley = size.h - 1;
//	bool reveal_enabled = gs->key_down_state(SDLK_BACKQUOTE);
	for (int y = min_tiley; y <= max_tiley; y++) {
		for (int x = min_tilex; x <= max_tilex; x++) {
			Tile& tile = get(Pos(x, y));
			const ldraw::Image& img = res::tile(tile.tile).img(tile.subtile);
			if (/*reveal_enabled ||*/is_seen(Pos(x, y))) {
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

	Size size = this->size();

	for (int i = 0; i < players.size(); i++) {
		PlayerInst* player = players[i];
		fov& f = player->field_of_view();
		BBox fovbox = f.tiles_covered();
		for (int y = std::max(fovbox.y1, 0);
				y <= std::min(fovbox.y2, size.h - 1); y++) {
			for (int x = std::max(fovbox.x1, 0);
					x <= std::min(fovbox.x2, size.w - 1); x++) {
				f.matches(x, y, matches);
				for (int i = 0; i < sub_sqrs * sub_sqrs; i++) {
					if (matches[i])
						_tiles[Pos(x,y)].seen = 1;
				}
			}
		}
	}
}
void GameTiles::post_draw(GameState* gs) {

	Size size = this->size();
	GameView& view = gs->view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	if (max_tilex >= size.w)
		max_tilex = size.w - 1;
	if (max_tiley >= size.h)
		max_tiley = size.h - 1;
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
			Tile& tile = get(Pos(x, y));

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
					if (!is_seen(Pos(x, y))) {
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
	serializer.write(size());
	serializer.write_container(_tiles._internal_vector());
	serializer.write_container(_solidity->_internal_vector());
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

bool GameTiles::radius_test(const Pos& xy, int rad, bool issolid, int ttype,
		Pos* hitloc) {
	int distsqr = (TILE_SIZE / 2 + rad), radsqr = rad * rad;
	distsqr *= distsqr; //sqr it

	Size size = this->size();

	int x = xy.x, y = xy.y;

	int mingrid_x = (x - rad) / TILE_SIZE, mingrid_y = (y - rad) / TILE_SIZE;
	int maxgrid_x = (x + rad) / TILE_SIZE, maxgrid_y = (y + rad) / TILE_SIZE;
	int minx = squish(mingrid_x, 0, size.w), miny = squish(mingrid_y, 0, size.h);
	int maxx = squish(maxgrid_x, 0, size.w), maxy = squish(maxgrid_y, 0, size.h);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int idx = yy * size.w + xx;

			TileState& tilestate = _tiles.raw_get(idx);
			Tile& tile = tilestate.tile;

			bool istype = (tile.tile == ttype || ttype == -1);
			bool solidity_match = _solidity->raw_get(idx) == issolid;

			if (solidity_match && istype) {
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
	Size newsize;
	serializer.read(newsize);
	_tiles.resize(newsize);
	_solidity->resize(newsize);
	serializer.read_container(_tiles._internal_vector());
	serializer.read_container(_solidity->_internal_vector());
}

