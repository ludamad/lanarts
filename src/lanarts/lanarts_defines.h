/* lanarts_defines.h
 *  Defines simple structures that are used throughout the code
 */

#ifndef LANARTS_DEFINES_H_
#define LANARTS_DEFINES_H_

#include <common/lcommon_defines.h>
#include <common/func_timer.h>
#include <common/range.h>

#include <cassert>
#include <cstdlib>

// The preferred assert statement to use, for ease of re-implementation
#define LANARTS_ASSERT(x) assert(x)

//TODO remove from lanarts_defines.h
/*Represents a single square tile*/
struct Tile {
	unsigned short tile, subtile;
	Tile(int tile = 0, int subtile = 0) :
			tile(tile), subtile(subtile) {
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
