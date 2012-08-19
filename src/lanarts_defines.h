/* lanarts_defines.h
 *  Defines simple structures that are used throughout the code
 */

#ifndef LANARTS_DEFINES_H_
#define LANARTS_DEFINES_H_

#include <cassert>
#include <cstdlib>

//These file should be included if there are issues with Microsoft's compiler
#ifdef _MSC_VER
#define snprintf _snprintf
#define round(x) floor((x)+0.5f)
#endif

#define LANARTS_ASSERT(x) assert(x)

/*Represents a Canadian colour*/
struct Colour {
	int r, g, b, a;
	Colour(int r = 255, int g = 255, int b = 255, int a = 255) :
			r(r), g(g), b(b), a(a) {
	}
	bool operator==(const Colour& col) const {
		return r == col.r && g == col.g && b == col.b && a == col.a;
	}
	Colour with_alpha(int alpha) const;
	Colour scale(float val) const;

	Colour inverse() const;
	Colour mute_colour(float val) const;
	Colour mult_alpha(float alpha) const;
	Colour multiply(const Colour& o) const;
};

/*Represents a width & height*/
struct Dim {
	int w, h;
	explicit Dim(int w = 0, int h = 0) :
			w(w), h(h) {
		LANARTS_ASSERT(w >= 0 && h >= 0);
	}
};

/*Represents a range*/
struct Range {
	int min, max;
	explicit Range(int min = 0, int max = 0) :
			min(min), max(max) {
		LANARTS_ASSERT(min <= max);
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

/*Represents a float x,y pair position*/
struct Posf {
	float x, y;
	Posf() :
			x(0.0f), y(0.0f) {
	}
	Posf(float x, float y) :
			x(x), y(y) {
	}
};

/*Represents a rectangular region in terms of its start and end x & y values*/
struct BBox {
	int x1, y1, x2, y2;
	BBox(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) :
			x1(x1), y1(y1), x2(x2), y2(y2) {
		LANARTS_ASSERT(x1 <= x2 && y1 <= y2);
	}
	bool contains(int x, int y) const {
		return x >= x1 && x < x2 && y >= y1 && y < y2;
	}
	bool contains(const Pos& p) const {
		return contains(p.x, p.y);
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
	BBox translated(int x, int y) const {
		return BBox(x1 + x, y1 + y, x2 + x, y2 + y);
	}
};

#define FOR_EACH_BBOX(bbox, x, y) \
	for (int y = (bbox).y1; y < (bbox).y2; y++)\
		for (int x = (bbox).x1; x < (bbox).x2; x++)

//TODO remove from lanarts_defines.h
/*Represents a single square tile*/
struct Tile {
	unsigned short tile, subtile;
	Tile(int tile = 0, int subtile = 0) :
			tile(tile), subtile(subtile) {
	}
};

//TODO replace region with BBox everywhere
/*Represents a rectangular region in terms of its start x,y values as well as its dimensions*/
struct Region {
	int x, y, w, h;
	Region(int x = 0, int y = 0, int w = 0, int h = 0) :
			x(x), y(y), w(w), h(h) {
	}
	inline Pos pos() {
		return Pos(x, y);
	}
	Region remove_perimeter(int p = 1) {
		return Region(x + p, y + p, w - 2 * p, h - 2 * p);
	}

};

static const int TILE_SIZE = 32;

/*Used to identify objects. Safe to store and use later, can query if object still in play.*/
typedef int obj_id;
const int NONE = 0;

/*Indices to the various game data arrays*/
typedef int class_id;
typedef int item_id;
typedef int itemgenlist_id;
typedef int armour_id;
typedef int effect_id;
typedef int projectile_id;
typedef int sprite_id;
typedef int spell_id;
typedef int scriptobj_id;
typedef int tile_id;
typedef int tileset_id;
typedef int enemy_id;
typedef int weapon_id;
/*Determines what team something belongs to*/
typedef int team_id;
typedef int level_id;

/*Type used to store monetary values*/
typedef int money_t;

//TODO: Does this really make sense here ?
/* Cooldown related utility function */
inline void cooldown_step(int& cooldown) {
	cooldown = cooldown > 0 ? cooldown - 1 : 0;
}

#endif
