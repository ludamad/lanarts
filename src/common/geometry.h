/*
 * geometry.h:
 *  Simple geometric defines
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

/*Represents a width & height*/
struct Dim {
	int w, h;
	explicit Dim(int w = 0, int h = 0);
	bool operator==(const Dim& o) const;
	bool operator!=(const Dim& o) const;
};

/*Represents an integer x,y pair position*/
struct Pos {
	int x, y;
	Pos();
	explicit Pos(const Dim& dim);
	bool operator==(const Pos& o) const;
	bool operator!=(const Pos& o) const;
	Pos(int x, int y);
};

/*Represents a rectangular region in terms of its start and end x & y values*/
struct BBox {
	int x1, y1, x2, y2;

	BBox();
	BBox(const Pos& pos, const Dim& dim);
	BBox(int x1, int y1, int x2, int y2);
	bool contains(int x, int y) const;
	bool contains(const Pos& p) const;

	bool operator==(const BBox& bbox) const;
	bool operator!=(const BBox& o) const;
	int width() const;
	int height() const;

	Pos left_top() const;
	Pos center() const;
	Dim size() const;
	int center_x() const;
	int center_y() const;
	void translate(int x, int y);
	bool empty() const;
	BBox translated(int x, int y) const;

	BBox translated(const Pos& pos) const;
};

#define FOR_EACH_BBOX(bbox, x, y) \
	for (int y = (bbox).y1; y < (bbox).y2; y++)\
		for (int x = (bbox).x1; x < (bbox).x2; x++)

//Float versions of above structures

/*Represents a width & heigh, with floats t*/
struct DimF {
	float w, h;
	explicit DimF(float w = 0, float h = 0);
	DimF(const Dim& dim);
	bool operator==(const DimF& o) const;
	bool operator!=(const DimF& o) const;
};

/*Represents a float x,y pair position*/
struct Posf {
	float x, y;
	Posf();
	Posf(float x, float y);
	explicit Posf(const DimF& dim);
	Posf(const Pos& pos);
	void rotate(float angle);
	Posf rotated(float angle) const;

	bool operator==(const Posf& o) const;
	bool operator!=(const Posf& o) const;
};
/*Represents a rectangular region in terms of its start and end x & y values, with floats*/
struct BBoxF {
	float x1, y1, x2, y2;
	BBoxF();
	BBoxF(float x1, float y1, float x2, float y2);
	BBoxF(const Posf& pos, const DimF& dim);
	BBoxF(const BBox& bbox);
	bool operator==(const BBoxF& bbox) const;
	bool operator!=(const BBoxF& o) const;
	bool contains(float x, float y) const;
	bool contains(const Posf& p) const;

	BBoxF subregion(const BBoxF& bboxf) const;

	float width() const;
	float height() const;
	DimF size() const;
	BBoxF scaled(float w, float h) const;

	BBoxF scaled(const DimF& scale) const;
	Posf left_top() const;

	Posf center() const;
	float center_x() const;
	float center_y() const;
	void translate(float x, float y);
	bool empty() const;
	BBoxF translated(float x, float y) const;

	BBoxF translated(const Posf& pos) const;
};

struct QuadF {
	Posf pos[4];

	QuadF() {
	}
	QuadF(const BBoxF& bbox, float angle = 0.0f);

	void rotate(float angle);
	QuadF rotated(float angle) const;
	void translate(const Posf& p);
	QuadF translated(const Posf& p) const;
};

/* Convenience operators for working with positions */
Pos operator+(const Pos& p1, const Pos& p2);
void operator+=(Pos& p1, const Pos& p2);

Pos operator-(const Pos& p1, const Pos& p2);
void operator-=(Pos& p1, const Pos& p2);

Dim operator+(const Dim& p1, const Dim& p2);
void operator+=(Dim& p1, const Dim& p2);

Dim operator-(const Dim& p1, const Dim& p2);
void operator-=(Dim& p1, const Dim& p2);

/* Floating point versions of the above */
Posf operator+(const Posf& p1, const Posf& p2);
void operator+=(Posf& p1, const Posf& p2);

Posf operator-(const Posf& p1, const Posf& p2);
void operator-=(Posf& p1, const Posf& p2);

DimF operator+(const DimF& p1, const DimF& p2);
void operator+=(DimF& p1, const DimF& p2);

DimF operator-(const DimF& p1, const DimF& p2);
void operator-=(DimF& p1, const DimF& p2);

#endif /* GEOMETRY_H_ */
