/*
 * geometry.h:
 *  Simple geometric defines
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include "lcommon_defines.h"

/*Represents a width & height*/
struct Dim {
	int w, h;
	explicit Dim(int w = 0, int h = 0) :
			w(w), h(h) {
		LCOMMON_ASSERT(w >= 0 && h >= 0);
	}
};

/*Represents an integer x,y pair position*/
struct Pos {
	int x, y;
	Pos() :
			x(0), y(0) {
	}
	bool operator==(const Pos& o) const {
		return o.x == x && o.y == y;
	}
	Pos(int x, int y) :
			x(x), y(y) {
	}

	void operator+=(const Pos& p) {
		x += p.x, y += p.y;
	}
	Pos operator+(const Pos& p) const {
		Pos ret(*this);
		ret += p;
		return ret;
	}
};

/*Represents a rectangular region in terms of its start and end x & y values*/
struct BBox {
	int x1, y1, x2, y2;
	BBox(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) :
			x1(x1), y1(y1), x2(x2), y2(y2) {
		LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
	}
	bool contains(int x, int y) const {
		return x >= x1 && x < x2 && y >= y1 && y < y2;
	}
	bool contains(const Pos& p) const {
		return contains(p.x, p.y);
	}

	bool operator==(const BBox& bbox) const {
		return x1 == bbox.x1 && y1 == bbox.y1 && x2 == bbox.x2 && y2 == bbox.y2;
	}

	int width() const {
		return x2 - x1;
	}
	int height() const {
		return y2 - y1;
	}
	Dim size() const {
		return Dim(width(), height());
	}
	int center_x() const {
		return (x1 + x2) / 2;
	}
	int center_y() const {
		return (y1 + y2) / 2;
	}
	void translate(int x, int y) {
		x1 += x, x2 += x;
		y1 += y, y2 += y;
	}
	bool empty() const {
		return x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0;
	}
	BBox translated(int x, int y) const {
		return BBox(x1 + x, y1 + y, x2 + x, y2 + y);
	}

	BBox translated(const Pos& pos) const {
		return BBox(x1 + pos.x, y1 + pos.y, x2 + pos.x, y2 + pos.y);
	}
};

#define FOR_EACH_BBOX(bbox, x, y) \
	for (int y = (bbox).y1; y < (bbox).y2; y++)\
		for (int x = (bbox).x1; x < (bbox).x2; x++)

//Float versions of above structures

/*Represents a float x,y pair position*/
struct Posf {
	float x, y;
	Posf() :
			x(0.0f), y(0.0f) {
	}
	Posf(float x, float y) :
			x(x), y(y) {
	}
	Posf(const Pos& pos) :
			x(pos.x), y(pos.y) {
	}
};

/*Represents a width & heigh, with floats t*/
struct DimF {
	float w, h;
	explicit DimF(float w = 0, float h = 0) :
			w(w), h(h) {
		LCOMMON_ASSERT(w >= 0 && h >= 0);
	}
	DimF(const Dim& dim) :
			w(dim.w), h(dim.h) {
		LCOMMON_ASSERT(w >= 0 && h >= 0);
	}
};

/*Represents a rectangular region in terms of its start and end x & y values, with floats*/
struct BBoxF {
	float x1, y1, x2, y2;
	BBoxF(float x1 = 0, float y1 = 0, float x2 = 0, float y2 = 0) :
			x1(x1), y1(y1), x2(x2), y2(y2) {
		LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
	}
	BBoxF(const BBoxF& bbox) :
			x1(bbox.x1), y1(bbox.y1), x2(bbox.x2), y2(bbox.y2) {
		LCOMMON_ASSERT(x1 <= x2 && y1 <= y2);
	}
	bool operator==(const BBoxF& bbox) const {
		return x1 == bbox.x1 && y1 == bbox.y1 && x2 == bbox.x2 && y2 == bbox.y2;
	}
	bool contains(float x, float y) const {
		return x >= x1 && x < x2 && y >= y1 && y < y2;
	}
	bool contains(const Posf& p) const {
		return contains(p.x, p.y);
	}

	float width() const {
		return x2 - x1;
	}
	float height() const {
		return y2 - y1;
	}
	DimF size() const {
		return DimF(width(), height());
	}
	BBoxF scaled(float w, float h) const {
		return BBoxF(x1 * w, y1 * h, x2 * w, y2 * h);
	}
	float center_x() const {
		return (x1 + x2) / 2;
	}
	float center_y() const {
		return (y1 + y2) / 2;
	}
	void translate(float x, float y) {
		x1 += x, x2 += x;
		y1 += y, y2 += y;
	}
	bool empty() const {
		return x1 == 0.0f && y1 == 0.0f && x2 == 0.0f && y2 == 0.0f;
	}
	BBoxF translated(float x, float y) const {
		return BBoxF(x1 + x, y1 + y, x2 + x, y2 + y);
	}

	BBoxF translated(const Posf& pos) const {
		return BBoxF(x1 + pos.x, y1 + pos.y, x2 + pos.x, y2 + pos.y);
	}
};

#endif /* GEOMETRY_H_ */
