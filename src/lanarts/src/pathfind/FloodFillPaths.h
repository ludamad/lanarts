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
	bool solid, open, marked;
	int dx, dy, distance;
	FloodFillNode() {
	}
	FloodFillNode(bool solid, bool open, int dx, int dy, int dist) :
			solid(solid), open(open), marked(false), dx(dx), dy(dy), distance(
					dist) {
	}
};

class FloodFillPaths {
public:
	FloodFillPaths(BoolGridRef solidity = BoolGridRef());
	~FloodFillPaths();

//Floodfill based API
	void calculate_path(GameState* gs, int ox, int oy, int radius);
	//Towards object
	void interpolated_direction(int x, int y, int w, int h, float speed,
			float& vx, float& vy, bool lenient = false);
	void interpolated_direction(const BBox& bbox, float speed, float& vx,
			float& vy) {
		interpolated_direction(bbox.x1, bbox.y1, bbox.width(), bbox.height(),
				speed, vx, vy);
	}
	//Away from object
	void random_further_direction(MTwist& mt, int x, int y, int w, int h,
			float speed, float& vx, float& vy);

	FloodFillNode* get(int x, int y) {
		LANARTS_ASSERT( x >= 0 && x < _size.w );
		LANARTS_ASSERT( y >= 0 && y < _size.h );
		return &path[Pos(x, y)];
	}

	BBox location() {
		return BBox(_source.x, _source.y, _source.x + _size.w, _source.y + _size.h);
	}

	int width() const {
		return _size.w;
	}
	int height() const {
		return _size.h;
	}

	void debug_draw(GameState* gs);
private:
	void point_to_local_min(int sx, int sy);
	void point_to_random_further(MTwist& mt, int sx, int sy);
	bool can_head(int sx, int sy, int ex, int ey, int speed, int dx, int dy);


	/* Shared with game tile structure! */
	BoolGridRef _solidity;

	Grid<FloodFillNode> path;
	/* The source point of the flood fill */
	Pos _source;
	/* Grid has own internal size, this is size actually used */
	Size _size;
};
#endif /* FLOODFILLPATHS_H_ */
