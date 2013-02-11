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

WanderMap::WanderMap(BoolGridRef solidity, const Size& size,
		const Size& division_size) {

	int source_w = round_up_divide(size.w, division_size.w);
	int source_h = round_up_divide(size.h, division_size.h);

	_frame = 0;
	_squares.resize(size);
	_sources.resize(Size(source_w, source_h));
	_division_size = division_size;
}

WanderMap::~WanderMap() {
}

WanderMapSquare& WanderMap::get(const Pos& xy) {
	return _squares[xy];
}

void WanderMap::step() {
	_frame++;
}

void WanderMap::generate_wander_map_section(const Pos& source_xy) {

}

void WanderMap::ensure_square_valid(const Pos& xy) {
	Pos source_xy = Pos(xy.x / _division_size.w, xy.y / _division_size.h);
	WanderMapSource& source = _sources[source_xy];
	if (source.expiry_frame >= _frame) {
		generate_wander_map_section(source_xy);
	}
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
