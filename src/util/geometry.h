/*
 * geometry.h
 *
 *  Created on: Mar 28, 2012
 *      Author: 100397561
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_
#include <cmath>

struct Pos {
	int x, y;
	Pos() {}
	bool operator==(const Pos& o) const {
		return o.x == x && o.y == y;
	}
	Pos(int x, int y) :
			x(x), y(y) {
	}
};

struct Region {
	int x, y, w, h;
	Region(int x, int y, int w, int h) :
			x(x), y(y), w(w), h(h) {
	}
	inline Pos pos() {
		return Pos(x, y);
	}
};



inline void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry, float speed = 1){
	rx = b.x - a.x;
	ry = b.y - a.y;
	float mag = sqrt(rx*rx + ry*ry);
	if (mag > 0){
		rx /= mag/speed;
		ry /= mag/speed;
	}
}

#endif /* GEOMETRY_H_ */
