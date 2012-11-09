/* lanarts_defines.h
 *  Defines simple structures that are used throughout the code
 */

#ifndef LANARTS_DEFINES_H_
#define LANARTS_DEFINES_H_

#include <common/geometry.h>
#include <common/lcommon_defines.h>
#include <common/PerfTimer.h>
#include <common/range.h>

#include <cassert>
#include <cstdlib>

// If this file is not included there may be issues with Microsoft's compiler
#ifdef _MSC_VER
#define snprintf _snprintf
#define round(x) floor((x)+0.5f)
#endif

// The preferred assert statement to use, for ease of re-implementation
#define LANARTS_ASSERT(x) assert(x)

// Define a cross-platform function name identifier
#ifdef _MSC_VER
#define FUNCNAME __FUNCSIG__
#else
#ifdef __GNUC__
#define FUNCNAME __PRETTY_FUNCTION__
#else
#define FUNCNAME __func__
#endif
#endif

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
const item_id NO_ITEM = -1;

typedef int area_id;
typedef int areatemplate_id;
typedef int itemgenlist_id;
typedef int armour_id;
typedef int effect_id;
typedef int equipment_id;
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
