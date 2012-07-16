/*
 * pathfind.h:
 *  Utilities for flood-fill based pathfinding
 */

#ifndef PATHFIND_H_
#define PATHFIND_H_

#include <vector>

#include "../util/game_basic_structs.h"
#include "../util/mtwist.h"

class GameState;

struct PathCoord {
	int x, y;
	int distance;
	PathCoord() {
	}
	PathCoord(short x, short y, int dist) :
			x(x), y(y), distance(dist) {
	}
	void operator=(const PathCoord& o) {
		x = o.x, y = o.y, distance = o.distance;
	}
	bool operator<(const PathCoord& o) const {
		if (distance == o.distance) {
			return this > &o; // Strict weak ordering
		}
		return distance > o.distance;
	}
};

struct PathingNode {
	bool solid, open, marked;
	int dx, dy, distance;
	PathingNode() {
	}
	PathingNode(bool solid, bool open, int dx, int dy, int dist) :
			solid(solid), open(open), marked(false), dx(dx), dy(dy), distance(
					dist) {
	}
};

//Used for floodfill or AStar
class PathInfo {
public:
	PathInfo();
	~PathInfo();

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
			float& vx, float& vy);
	void interpolated_direction(const BBox& bbox, float speed, float& vx,
			float& vy) {
		interpolated_direction(bbox.x1, bbox.y1, bbox.width(), bbox.height(),
				speed, vx, vy);
	}
	//Away from object
	void random_further_direction(MTwist& mt, int x, int y, int w, int h,
			float speed, float& vx, float& vy);
	PathingNode* get(int x, int y) {
		LANARTS_ASSERT( x >= 0 && x < w );
		LANARTS_ASSERT( y >= 0 && y < h );
		return &path[alloc_w * y + x];
	}
	void stake_claim(int x, int y);
	//Call before 'interpolated_direction'
	void adjust_for_claims(int x, int y);

	void draw(GameState* gs);
private:
	void point_to_local_min(int sx, int sy);
	void point_to_random_further(MTwist& mt, int sx, int sy);
	void fix_distances(int sx, int sy);
	bool can_head(int sx, int sy, int ex, int ey, int speed, int dx, int dy);
	PathingNode* path;
	int start_x, start_y;
	int path_x, path_y;
	int w, h;
	int alloc_w, alloc_h;
};

void floodfill(PathingNode* path, int w, int h, int sx, int sy);

#endif /* PATHFIND_H_ */
