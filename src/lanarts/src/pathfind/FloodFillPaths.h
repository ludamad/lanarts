/*
 * FloodFillPaths.h:
 *  Utilities for flood-fill based pathfinding
 */

#ifndef FLOODFILLPATHS_H_
#define FLOODFILLPATHS_H_

#include <vector>

#include <lcommon/mtwist.h>

#include "util/Grid.h"

#include "SolidityGridRef.h"

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
	FloodFillPaths();
	~FloodFillPaths();

	int width() {
		return w;
	}
	int height() {
		return h;
	}

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
		LANARTS_ASSERT( x >= 0 && x < w );
		LANARTS_ASSERT( y >= 0 && y < h );
		return &path[alloc_w * y + x];
	}

	BBox location() {
		return BBox(start_x, start_y, start_x + w, start_y + h);
	}

	void debug_draw(GameState* gs);
private:
	void point_to_local_min(int sx, int sy);
	void point_to_random_further(MTwist& mt, int sx, int sy);
	void fix_distances(int sx, int sy);
	bool can_head(int sx, int sy, int ex, int ey, int speed, int dx, int dy);

	FloodFillNode* path;
	int start_x, start_y;
	int path_x, path_y;
	int w, h;
	int alloc_w, alloc_h;
};

void floodfill(FloodFillNode* path, int w, int h, int sx, int sy);

#endif /* FLOODFILLPATHS_H_ */
