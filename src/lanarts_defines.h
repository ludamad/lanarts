/* lanarts_defines.h
 *  Defines simple structures that are used throughout the code
 */

#ifndef LANARTS_DEFINES_H_
#define LANARTS_DEFINES_H_

#include <lcommon/perf_timer.h>
#include <lcommon/Range.h>

#include <cassert>
#include <cstdlib>

// The preferred assert statement to use, for ease of re-implementation
#define LANARTS_ASSERT(x) assert(x)

#ifdef _MSC_VER
#define round(x) floor(x + 0.5f)
#define snprintf _snprintf
#endif

static const int TILE_SIZE = 32;

/*Used to identify objects. Safe to store and use later, can query if object still in play.*/
typedef int obj_id;
const int NONE = 0;

/*Indices to the various game data arrays*/
typedef int class_id;
typedef int item_id;
const item_id NO_ITEM = -1;

typedef int area_id;
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
typedef int level_id;

/*Type used to store monetary values*/
typedef int money_t;

//TODO: Does this really make sense here ?
/* Cooldown related utility function */
inline void cooldown_step(int& cooldown) {
	cooldown = cooldown > 0 ? cooldown - 1 : 0;
}

// This is required to ensure we do proper cleanup without hanging on exit
// This does NOT abort
void lanarts_system_quit();

#endif
