/*
 * geometry.cpp:
 *  Simple geometric defines
 */

#include <cmath>

#include "lcommon_assert.h"
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

BBox remove_perimeter(const BBox& bbox, int p = 1) {
	return BBox(bbox.x1 + p, bbox.y1 + p, bbox.x2 - p * 2, bbox.y2 - p * 2);
}

Dim::Dim(int w, int h) :
		w(w), h(h) {
	LCOMMON_ASSERT(w >= 0 && h >= 0);
}

bool Dim::operator ==(const Dim & o) const {
	return o.w == w && o.h == h;
}

bool Dim::operator !=(const Dim & o) const {
	return !(*this == o);
}

Pos::Pos() :
		x(0), y(0) {
}

Pos::Pos(const Dim & dim) :
		x(dim.w), y(dim.h) {
}

bool Pos::operator ==(const Pos & o) const {
	return o.x == x && o.y == y;
}

bool Pos::operator !=(const Pos & o) const {
	return !(*this == o);
}

Pos::Pos(int x, int y) :
		x(x), y(y) {
}

BBox::BBox() :
		x1(0), y1(0), x2(0), y2(0) {
}

BBox::BBox(const Pos & pos, const Dim & dim) :
		x1(pos.x), y1(pos.y), x2(pos.x + dim.w), y2(pos.y + dim.h) {
	LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
}

BBox::BBox(int x1, int y1, int x2, int y2) :
		x1(x1), y1(y1), x2(x2), y2(y2) {
	LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
}

bool BBox::contains(int x, int y) const {
	return x >= x1 && x < x2 && y >= y1 && y < y2;
}

bool BBox::contains(const Pos & p) const {
	return contains(p.x, p.y);
}

bool BBox::operator ==(const BBox & bbox) const {
	return x1 == bbox.x1 && y1 == bbox.y1 && x2 == bbox.x2 && y2 == bbox.y2;
}

bool BBox::operator !=(const BBox & o) const {
	return !(*this == o);
}

int BBox::width() const {
	return x2 - x1;
}

int BBox::height() const {
	return y2 - y1;
}

Pos BBox::left_top() const {
	return Pos(x1, y1);
}

Pos BBox::center() const {
	return Pos((x1 + x2) / 2, (y1 + y2) / 2);
}

Dim BBox::size() const {
	return Dim(width(), height());
}

int BBox::center_x() const {
	return (x1 + x2) / 2;
}

int BBox::center_y() const {
	return (y1 + y2) / 2;
}

void BBox::translate(int x, int y) {
	x1 += x, x2 += x;
	y1 += y, y2 += y;
}

bool BBox::empty() const {
	return x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0;
}

BBox BBox::translated(int x, int y) const {
	return BBox(x1 + x, y1 + y, x2 + x, y2 + y);
}

BBox BBox::translated(const Pos & pos) const {
	return BBox(x1 + pos.x, y1 + pos.y, x2 + pos.x, y2 + pos.y);
}

DimF::DimF(float w, float h) :
		w(w), h(h) {
	LCOMMON_ASSERT(w >= 0 && h >= 0);
}

DimF::DimF(const Dim & dim) :
		w(dim.w), h(dim.h) {
	LCOMMON_ASSERT(w >= 0 && h >= 0);
}

bool DimF::operator ==(const DimF & o) const {
	return o.w == w && o.h == h;
}

bool DimF::operator !=(const DimF & o) const {
	return !(*this == o);
}

Posf::Posf() :
		x(0.0f), y(0.0f) {
}

Posf::Posf(float x, float y) :
		x(x), y(y) {
}

Posf::Posf(const DimF & dim) :
		x(dim.w), y(dim.h) {
}

Posf::Posf(const Pos & pos) :
		x(pos.x), y(pos.y) {
}

bool Posf::operator ==(const Posf & o) const {
	return o.x == x && o.y == y;
}

bool Posf::operator !=(const Posf & o) const {
	return !(*this == o);
}

BBoxF::BBoxF() :
		x1(0), y1(0), x2(0), y2(0) {
}

BBoxF::BBoxF(float x1, float y1, float x2, float y2) :
		x1(x1), y1(y1), x2(x2), y2(y2) {
	LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
}

BBoxF::BBoxF(const Posf & pos, const DimF & dim) :
		x1(pos.x), y1(pos.y), x2(pos.x + dim.w), y2(pos.y + dim.h) {
	LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
}

BBoxF::BBoxF(const BBox & bbox) :
		x1(bbox.x1), y1(bbox.y1), x2(bbox.x2), y2(bbox.y2) {
	LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
}

bool BBoxF::operator ==(const BBoxF & bbox) const {
	return x1 == bbox.x1 && y1 == bbox.y1 && x2 == bbox.x2 && y2 == bbox.y2;
}

bool BBoxF::operator !=(const BBoxF & o) const {
	return !(*this == o);
}

bool BBoxF::contains(float x, float y) const {
	return x >= x1 && x < x2 && y >= y1 && y < y2;
}

bool BBoxF::contains(const Posf & p) const {
	return contains(p.x, p.y);
}

BBoxF BBoxF::subregion(const BBoxF & bboxf) const {
	int w = width(), h = height();
	return BBoxF(x1 + w * bboxf.x1, y1 + h * bboxf.y1, x1 + w * bboxf.x2,
			y1 + h * bboxf.y2);
}

float BBoxF::width() const {
	return x2 - x1;
}

float BBoxF::height() const {
	return y2 - y1;
}

DimF BBoxF::size() const {
	return DimF(width(), height());
}

BBoxF BBoxF::scaled(float w, float h) const {
	return BBoxF(x1 * w, y1 * h, x2 * w, y2 * h);
}

BBoxF BBoxF::scaled(const DimF & scale) const {
	return scaled(scale.w, scale.h);
}

Posf BBoxF::left_top() const {
	return Posf(x1, y1);
}

Posf BBoxF::center() const {
	return Posf((x1 + x2) / 2, (y1 + y2) / 2);
}

float BBoxF::center_x() const {
	return (x1 + x2) / 2;
}

float BBoxF::center_y() const {
	return (y1 + y2) / 2;
}

void BBoxF::translate(float x, float y) {
	x1 += x, x2 += x;
	y1 += y, y2 += y;
}

bool BBoxF::empty() const {
	return x1 == 0.0f && y1 == 0.0f && x2 == 0.0f && y2 == 0.0f;
}

BBoxF BBoxF::translated(float x, float y) const {
	return BBoxF(x1 + x, y1 + y, x2 + x, y2 + y);
}

BBoxF BBoxF::translated(const Posf & pos) const {
	return BBoxF(x1 + pos.x, y1 + pos.y, x2 + pos.x, y2 + pos.y);
}
