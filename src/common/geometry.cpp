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

// Convenience operators:

Pos operator +(const Pos & p1, const Pos & p2) {
	return Pos(p1.x + p2.x, p1.y + p2.y);
}

void operator +=(Pos & p1, const Pos & p2) {
	p1.x += p2.x;
	p1.y += p2.y;
}

Pos operator -(const Pos & p1, const Pos & p2) {
	return Pos(p1.x - p2.x, p1.y - p2.y);
}

void operator -=(Pos & p1, const Pos & p2) {
	p1 = p1 - p2;
}

Dim operator +(const Dim & p1, const Dim & p2) {
	return Dim(p1.w + p2.w, p1.h + p2.h);
}

void operator +=(Dim & p1, const Dim & p2) {
	p1 = p1 + p2;
}

Dim operator -(const Dim & p1, const Dim & p2) {
	return Dim(p1.w - p2.w, p1.h - p2.h);
}

// Floating point versions of the above
Posf operator +(const Posf & p1, const Posf & p2) {
	return Posf(p1.x + p2.x, p1.y + p2.y);
}

void operator +=(Posf & p1, const Posf & p2) {
	p1 = p1 + p2;
}

Posf operator -(const Posf & p1, const Posf & p2) {
	return Posf(p1.x - p2.x, p1.y - p2.y);
}

void operator -=(Posf & p1, const Posf & p2) {
	p1 = p1 - p2;
}

DimF operator +(const DimF & p1, const DimF & p2) {
	return DimF(p1.w + p2.w, p1.h + p2.h);
}

void operator +=(DimF & p1, const DimF & p2) {
	p1 = p1 + p2;
}

DimF operator -(const DimF & p1, const DimF & p2) {
	return DimF(p1.w - p2.w, p1.h - p2.h);
}

