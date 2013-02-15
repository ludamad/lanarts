/*
 * FloodFillPaths.h:
 *  Utilities for flood-fill based pathfinding
 */

#ifndef FLOODFILLPATHS_H_
#define FLOODFILLPATHS_H_

#include <vector>

#include <lcommon/mtwist.h>

#include "util/Grid.h"

#include "BoolGridRef.h"

class GameState;

struct FloodFillCoord {
	int x, y;
	int distance;
	FloodFillCoord() {
	}
	FloodFillCoord(short x, short y, int dist) :
			x(x), y(y), distance(dist) {
	}
	void operator=(const FloodFillCoord& o) {
		x = o.x, y = o.y, distance = o.distance;
	}
	bool operator<(const FloodFillCoord& o) const {
		if (distance == o.distance) {
			return this > &o; // Strict weak ordering
		}
		return distance > o.distance;
	}
};

struct FloodFillNode {
	bool solid, open;
	int dx, dy, distance;
	FloodFillNode() {
	}
	FloodFillNode(bool solid, bool open, int dx, int dy, int dist) :
			solid(solid), open(open), dx(dx), dy(dy), distance(
					dist) {
	}
};

class FloodFillPaths {
public:
	FloodFillPaths(const BoolGridRef& solidity = BoolGridRef());
	~FloodFillPaths();
	void initialize(const BoolGridRef& solidity);

	void fill_paths_in_radius(const Pos& source_xy, int radius);

	void fill_paths_tile_region(const Pos& tile_xy, const BBox& area,
			bool clear_previous = true);

	//Towards object
	PosF interpolated_direction(const BBox& bbox, float speed, bool lenient =
			false);

	//Away from object
	PosF random_further_direction(MTwist& mt, int x, int y, int w, int h,
			float speed);

	BBox location() {
		return BBox(_topleft_xy.x, _topleft_xy.y, _topleft_xy.x + _size.w,
				_topleft_xy.y + _size.h);
	}

	FloodFillNode* node_at(const Pos& tile_xy) {
		return get(tile_xy.x - _topleft_xy.x, tile_xy.y - _topleft_xy.y);
	}

	Size size() const {
		return _size;
	}

	int width() const {
		return _size.w;
	}

	int height() const {
		return _size.h;
	}

	void debug_draw(GameState* gs);

private:
	FloodFillNode* get(int x, int y) {
		LANARTS_ASSERT( x >= 0 && x < _size.w);
		LANARTS_ASSERT( y >= 0 && y < _size.h);
		return &_path[Pos(x, y)];
	}
	bool is_solid_or_out_of_bounds(int x, int y);
	void point_to_local_min(int sx, int sy);
	void point_to_random_further(MTwist& mt, int sx, int sy);
	bool can_head(const BBox& bbox, int speed, int dx, int dy);

	/* Shared with game tile structure! */
	BoolGridRef _solidity;

	Grid<FloodFillNode> _path;
	/* The top left point covered by the flood fill */
	Pos _topleft_xy;
	/* Grid has own internal size, this is size actually used */
	Size _size;
};
#endif /* FLOODFILLPATHS_H_ */
