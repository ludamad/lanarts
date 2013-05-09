/*
 * WanderMap.cpp:
 *  Keeps a lazily-updated map of potential locations to wander.
 *  See WanderMap.h for details.
 */

#include <algorithm>

#include <lcommon/math_util.h>

#include "lanarts_defines.h"

#include "WanderMap.h"

// Fill with squares around the current square that do not go off the map
static void fill_with_surrounding_squares(std::vector<Pos>& squares,
		const Size& size, const Pos& source) {
	squares.clear();

	int min_x = std::max(source.x - 1, 0);
	int max_x = std::min(source.x + 1, size.w - 1);

	int min_y = std::max(source.y - 1, 0);
	int max_y = std::min(source.y + 1, size.h - 1);

	for (int y = min_y; y <= max_y; y++) {
		for (int x = min_x; x <= max_x; x++) {
			squares.push_back(Pos(x, y));
		}
	}
}

WanderMap::WanderMap(const BoolGridRef& solidity, const Size& size,
		const Size& division_size) {
	initialize(solidity, size, division_size);

	HACK_FOR_NOW_filled = false;
}

void WanderMap::initialize(const BoolGridRef& solidity, const Size& size,
		const Size& division_size) {
	_solidity = solidity;
	_division_size = division_size;

	_squares.resize(size);

	_cache.paths_from_source.initialize(_solidity);
}

WanderMap::~WanderMap() {
}

WanderMapSquare& WanderMap::get(const Pos& xy) {
	return _squares[xy];
}

void WanderMap::generate_wander_map_section(const Pos& source_xy) {
	FloodFillPaths& paths = _cache.paths_from_source;
	paths.initialize(_solidity);

	Pos corner(source_xy.x * _division_size.w, source_xy.y * _division_size.h);
	BBox region(corner, _division_size);

	// Should generate from center as much as possible,
	// but should also make sure each square is filled

	Pos center_xy = region.center();
	paths.fill_paths_tile_region(center_xy, region, true /* Clear initially */);

	// Ensure everything filled from center until bottom right
	Pos tile_xy = center_xy;
	for (; tile_xy.y < region.y2; tile_xy.y++) {
		for (; tile_xy.x < region.x2; tile_xy.x++) {
			FloodFillNode* node = paths.node_at(tile_xy);
			if (!(*_solidity)[tile_xy] || node->open) {
				paths.fill_paths_tile_region(tile_xy, region, false /* Incrementally fill */);
			}
			_squares[tile_xy].distance_to_source = node->distance;
		}
		tile_xy.x = region.x1;
	}

	// Ensure rest is filled
	for (tile_xy.y = region.y1; tile_xy.y < region.y2; tile_xy.y++) {
		for (tile_xy.x = region.x1; tile_xy.x < region.x2; tile_xy.x++) {

			// Back where we started, return
			if (tile_xy == center_xy) {
				return;
			}

			FloodFillNode* node = paths.node_at(tile_xy);
			if (!(*_solidity)[tile_xy] && node->open) {
				paths.fill_paths_tile_region(tile_xy, region, false /* Incrementally fill */);
			}
			_squares[tile_xy].distance_to_source = node->distance;
		}
	}
}

void WanderMap::ensure_filled() {
	if (HACK_FOR_NOW_filled) return;

	Size size = _squares.size();
	int source_w = round_up_divide(size.w, _division_size.w);
	int source_h = round_up_divide(size.h, _division_size.h);

	for (int y = 0; y < source_w; y++) {
		for (int x = 0; x < source_h; x++) {
			generate_wander_map_section(Pos(x,y));
		}
	}
	HACK_FOR_NOW_filled = true;
}

#include <ldraw/DrawOptions.h>
#include <ldraw/colour_constants.h>
#include "draw/fonts.h"

void WanderMap::debug_draw() {
	ensure_filled();

	Size size = _squares.size();

	ldraw::DrawOptions opt(ldraw::CENTER, COL_WHITE);

	for (int y = 0; y < size.w; y++) {
		for (int x = 0; x < size.h; x++) {
			Pos sqr_xy(x,y);
			Pos draw_xy = centered_multiple(sqr_xy, TILE_SIZE);
			res::font_primary().drawf(opt, draw_xy, "%d", (int)_squares[sqr_xy].distance_to_source);
		}
	}
}

void WanderMap::ensure_square_valid(const Pos& xy) {
	Pos source_xy = Pos(xy.x / _division_size.w, xy.y / _division_size.h);
	generate_wander_map_section(source_xy);
}

const std::vector<Pos>& WanderMap::candidates(const Pos& xy,
		WanderDirection direction) {
	std::vector<Pos>& candidates = _cache.candidates;

	fill_with_surrounding_squares(candidates, _squares.size(), xy);

	for (int i = 0; i < candidates.size(); i++) {
		ensure_square_valid(candidates[i]);
	}

	return candidates;
}
