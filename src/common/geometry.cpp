/*
 * geometry.cpp:
 *  Simple geometric defines
 */

#include <cmath>
#include "geometry.h"

void Posf::rotate(float angle) {
	*this = rotated(angle);
}

Posf Posf::rotated(float angle) const {
	float cosa = cos(angle), sina = sin(angle);
	return Posf(x * sina + y * cosa, -x * cosa + y * sina);
}

QuadF::QuadF(const BBoxF & bbox, float angle) {
	pos[0] = Posf(bbox.x1, bbox.y1);
	pos[1] = Posf(bbox.x2, bbox.y1);
	pos[2] = Posf(bbox.x2, bbox.y2);
	pos[3] = Posf(bbox.x1, bbox.y2);
	rotate(angle);
}
void QuadF::rotate(float angle) {
	if (angle == 0.0f) {
		return;
	}
	for (int i = 0; i < 4; i++) {
		pos[i] = pos[i].rotated(angle);
	}
}

QuadF QuadF::rotated(float angle) const {
	QuadF cpy(*this);
	cpy.rotate(angle);
	return cpy;
}

void QuadF::translate(const Posf & p) {
	for (int i = 0; i < 4; i++) {
		pos[i].x += p.x;
		pos[i].y += p.y;
	}
}

QuadF QuadF::translated(const Posf & p) const {
	QuadF cpy(*this);
	cpy.translate(p);
	return cpy;
}

