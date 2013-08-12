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

#include "objects/player/PlayerInst.h"

#include <lcommon/SerializeBuffer.h>

#include <lcommon/math_util.h>
#include "GameState.h"

#include "GameTiles.h"

GameTiles::GameTiles(const Size& size) :
		_tiles(size) {

	_solidity.set( new Grid<bool>(size, true) );
	_seen.set( new Grid<bool>(size, false) );
	_seethrough.set( new Grid<bool>(size, true) );
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
	return _tiles[xy];
}

bool GameTiles::is_seen(const Pos& xy) {
	return (*_seen)[xy];
}

void GameTiles::set_seethrough(const Pos& xy, bool seethrough) {
	(*_seethrough)[xy] = seethrough;
}

void GameTiles::set_solid(const Pos& xy, bool solid) {
	(*_solidity)[xy] = solid;
}

bool GameTiles::is_solid(const Pos& xy) {
	return (*_solidity)[xy];
}

bool GameTiles::is_seethrough(const Pos& xy) {
	return (*_seethrough)[xy];
}

void GameTiles::clear() {
	memset(_tiles.begin(), 0, sizeof(Tile) * size().area());
	_solidity->fill(false);
	_seen->fill(false);
	_seethrough->fill(false);
}

void GameTiles::mark_all_seen() {
	_seen->fill(true);
}

void GameTiles::copy_to(GameTiles & t) const {
	t._solidity = _solidity;
	t._tiles = _tiles;
}

void GameTiles::pre_draw(GameState* gs) {
	perf_timer_begin(FUNCNAME);

	Size size = this->size();
	GameView& view = gs->view();

	BBox region = view.tile_region_covered();

	if (region.x2 >= size.w) {
		region.x2 = size.w - 1;
	}
	if (region.y2 >= size.h) {
		region.y2 = size.h - 1;
	}

//	bool reveal_enabled = gs->key_down_state(SDLK_BACKQUOTE);
	for (int y = region.y1; y <= region.y2; y++) {
		for (int x = region.x1; x <= region.x2; x++) {
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
					if (matches[i]) {
						(*_seen)[Pos(x,y)] = true;
					}
				}
			}
		}
	}
}
void GameTiles::post_draw(GameState* gs) {

	Size size = this->size();
	GameView& view = gs->view();

	BBox region = view.tile_region_covered();

	if (region.x2 >= size.w) {
		region.x2 = size.w - 1;
	}
	if (region.y2 >= size.h) {
		region.y2 = size.h - 1;
	}

	const int sub_sqrs = VISION_SUBSQRS;

	if (/*gs->key_down_state(SDLK_BACKQUOTE) ||*/!gs->level_has_player()) {
		return;
	}
	perf_timer_begin(FUNCNAME);

	fov& mainfov = gs->local_player()->field_of_view();
	char matches[sub_sqrs * sub_sqrs];
	for (int y = region.y1; y <= region.y2; y++) {
		for (int x = region.x1; x <= region.x2; x++) {
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
	serializer.write_container(_seen->_internal_vector());
	serializer.write_container(_seethrough->_internal_vector());
}

//static bool circle_line_test(int px, int py, int qx, int qy, int cx, int cy,
//		float radsqr) {
//	int dx, dy, t, rt, ddist;
//	dx = qx - px;
//	dy = qy - py;
//	ddist = dx * dx + dy * dy;
//	t = -((px - cx) * dx + (py - cy) * dy);
//
//	/* Restrict t to within the limits of the line segment */
//	if (t < 0)
//		t = 0;
//	else if (t > ddist)
//		t = ddist;
//
//	dx = (px + t * (qx - px) / ddist) - cx;
//	dy = (py + t * (qy - py) / ddist) - cy;
//	rt = (dx * dx) + (dy * dy);
//	return rt < (radsqr);
//}

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

			Tile& tile = _tiles.raw_get(idx);

			bool istype = (tile.tile == ttype || ttype == -1);
			bool solidity_match = _solidity->raw_get(idx) == issolid;

			if (solidity_match && istype) {
				BBox tilebox(Pos(xx * TILE_SIZE, yy * TILE_SIZE), Size(TILE_SIZE, TILE_SIZE));
				Pos dist = tilebox.center() - xy;
				double ddist = dist.x * dist.x + dist.y * dist.y;
				if (ddist < distsqr || circle_rectangle_test(xy, rad, tilebox)) {
					if (hitloc)
						*hitloc = Pos(xx, yy);
					return true;
				}
			}
		}
	}
	return false;
}

void GameTiles::deserialize(SerializeBuffer& serializer) {
	Size newsize;
	serializer.read(newsize);

	_tiles.resize(newsize);
	_solidity->resize(newsize);
	_seen->resize(newsize);
	_seethrough->resize(newsize);

	serializer.read_container(_tiles._internal_vector());
	serializer.read_container(_solidity->_internal_vector());
	serializer.read_container(_seen->_internal_vector());
	serializer.read_container(_seethrough->_internal_vector());
}

BoolGridRef GameTiles::solidity_map() const {
	return _solidity;
}


BoolGridRef GameTiles::previously_seen_map() const {
	return _seen;
}


BoolGridRef GameTiles::seethrough_map() const {
	return _seethrough;
}
