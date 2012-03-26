/*
 * pathfind.h
 *
 *  Created on: Feb 6, 2012
 *      Author: 100397561
 */

#ifndef PATHFIND_H_
#define PATHFIND_H_
#include "../procedural/mtwist.h"

class GameState;

struct PathCoord {
	int x, y;
	int distance;
	PathCoord() {
	}
	PathCoord(short x, short y, int dist) :
			x(x), y(y), distance(dist) {
	}
	void operator=(const PathCoord& o){
		x = o.x, y = o.y, distance = o.distance;
	}
	bool operator<(const PathCoord& o) const{
		return distance > o.distance;
	}
};

struct PathingNode {
	bool solid, open, marked;
	int dx, dy, distance;
	PathingNode() {
	}
	PathingNode(bool solid, bool open, int dx, int dy, int dist) :
			solid(solid), open(open), marked(false), dx(dx), dy(dy), distance(dist) {
	}
};

class PathInfo {
public:
	PathInfo();
	~PathInfo();
	void calculate_path(GameState* gs, int ox, int oy, int radius);

	int width() { return w; }
	int height() { return h; }
	//Towards object
	void interpolated_direction(int x, int y, int w, int h, float speed, float& vx, float& vy);
	//Away from object
	void random_further_direction(MTwist& mt, int x, int y, int w, int h, float speed, float& vx, float& vy);
	PathingNode* get(int x, int y) { return &path[w*y+x]; }
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
	int w,h;
};



void floodfill(PathingNode* path, int w, int h, int sx, int sy);

#endif /* PATHFIND_H_ */
