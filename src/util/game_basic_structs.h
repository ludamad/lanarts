/* game_basic_structs.h
 *  Defines simple structures that are used throughout the code
 */

#ifndef GAME_BASIC_STRUCTS_H_
#define GAME_BASIC_STRUCTS_H_

#include <cassert>
#include <cstdlib>

/*Represents a Canadian colour*/
struct Colour {
	int r, g, b, a;
	Colour(int r = 255, int g = 255, int b = 255, int a = 255) :
			r(r), g(g), b(b), a(a) {
	}
	bool operator==(const Colour& col) const {
		return r == col.r && g == col.g && b == col.b && a == col.a;
	}
};

/*Represents a range*/
struct Range {
	int min, max;
	Range() : min(0), max(0) {
	}
	Range(int min, int max) :
			min(min), max(max) {
	}
	Range(const Range& r) {
		min = r.min, max = r.max;
	}
};

/*Represents a rectangular region in terms of its start and end x & y values*/
struct BBox {
	int x1, y1, x2, y2;
	BBox(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) :
			x1(x1), y1(y1), x2(x2), y2(y2) {
	}
	int width() const {
		return x2 - x1;
	}
	int height() const {
		return y2 - y1;
	}
	int center_x() const {
		return (x1 + x2) / 2;
	}
	int center_y() const {
		return (y1 + y2) / 2;
	}
	BBox translated(int x, int y) {
		return BBox(x1 + x, y1 + y, x2 + x, y2 + y);
	}
};

/*Represents a single square tile*/
struct Tile {
	unsigned short tile, subtile;
	Tile(int tile = 0, int subtile = 0) :
			tile(tile), subtile(subtile) {
	}
};

/*Represents an integer x,y pair position*/
struct Pos {
	int x, y;
	Pos() {
	}
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
/*Indices to the various game data arrays*/
typedef int class_id;
typedef int item_id;
typedef int armour_id;
typedef int projectile_id;
typedef int sprite_id;
typedef int spell_id;
typedef int tile_id;
typedef int tileset_id;
typedef int enemy_id;
typedef int weapon_id;
/*Determines what team something belongs to*/
typedef int team_id;

/*Type used to store monetary values*/
typedef int money_t;

const int NONE = 0;

#define LANARTS_ASSERT(x) assert(x)
#define FOR_EACH_BBOX(bbox, x, y) \
	for (int y = (bbox).y1; y < (bbox).y2; y++)\
		for (int x = (bbox).x1; x < (bbox).x2; x++)

#endif
