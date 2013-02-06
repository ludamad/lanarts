/*
 * WanderMap.cpp:
 *  Keeps a lazily-updated map of potential locations to wander.
 *  See WanderMap.h for details.
 */

#include "lanarts_defines.h"

#include "WanderMap.h"

// Fill with squares around the current square that do not go off the map
static void fill_with_surrounding_squares(const Size& size) {

}

WanderMap::WanderMap(const Size& size) {
	_size = size;
}

WanderMap::~WanderMap() {
}

WanderMapSquare& WanderMap::get(const Pos& xy) {
	LANARTS_ASSERT(xy.x >= 0 && xy.x < _size.w);
	LANARTS_ASSERT(xy.y >= 0 && xy.y < _size.h);

	return _squares[xy.y * _size.w + xy.x];
}

WanderMapSquare& WanderMap::get(const Pos& xy) {
	LANARTS_ASSERT(xy.x >= 0 && xy.x < _size.w);
	LANARTS_ASSERT(xy.y >= 0 && xy.y < _size.h);

	return _squares[xy.y * _size.w + xy.x];
}
