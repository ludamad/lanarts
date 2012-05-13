/* game_basic_structs.h
 *  Defines simple structures that are used throughout the code
 */

#ifndef GAME_BASIC_STRUCTS_H_
#define GAME_BASIC_STRUCTS_H_

/*Represents a Canadian colour*/
struct Colour {
    int r, g, b, a;
    Colour(int r = 255, int g = 255, int b = 255, int a = 255) :
        r(r), g(g), b(b), a(a) {}
};

/*Represents a rectangular region in terms of its start and end x & y values*/
struct BBox {
	int x1, y1, x2, y2;
	BBox(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) :
		x1(x1), y1(y1), x2(x2), y2(y2){}
	int width() const{
		return x2 - x1;
	}
	int height() const{
		return y2 - y1;
	}
	int center_x() const {
		return (x1 + x2)/2;
	}
	int center_y() const {
		return (y1 + y2)/2;
	}
};

/*Represents an integer x,y pair position*/
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

/*Represents a rectangular region in terms of its start x,y values as well as its dimensions*/
struct Region {
	int x, y, w, h;
	Region(int x, int y, int w, int h) :
			x(x), y(y), w(w), h(h) {
	}
	inline Pos pos() {
		return Pos(x, y);
	}
};


static const int TILE_SIZE = 32;

/*Used to identify objects. Safe to store and use later, can query if object still in play.*/
typedef int obj_id;

#define LANARTS_ASSERT(x) assert(x)
#define FOR_EACH_BBOX(bbox, x, y) \
	for (int y = (bbox).y1; y < (bbox).y2; y++)\
		for (int x = (bbox).x1; x < (bbox).x2; x++)



#endif
