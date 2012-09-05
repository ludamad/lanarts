/*
 * spell_data.h:
 *  Defines the data that goes into a spell
 */

#ifndef SPELL_DATA_H_
#define SPELL_DATA_H_

#include <string>
#include <vector>

#include "../lua/LuaValue.h"

#include "../lanarts_defines.h"

#include "items/items.h"

struct SpellEntry {
	std::string name, description;
	sprite_id sprite;
	int mp_cost, cooldown;
	LuaValue action_func; //Immediate action
	LuaValue autotarget_func; //Auto-target func
	LuaValue prereq_func; //Pre-req to casting
	Projectile projectile; //Projectile used, if any
	bool can_cast_with_cooldown, can_cast_with_held_key;
	SpellEntry() :
			sprite(-1), mp_cost(0), cooldown(0), can_cast_with_cooldown(false), can_cast_with_held_key(
					false) {
	}

	void init(lua_State* L) {
		action_func.initialize(L);
		autotarget_func.initialize(L);
		prereq_func.initialize(L);
	}
	bool uses_projectile() {
		return projectile.id != NO_ITEM;
	}
};

extern std::vector<SpellEntry> game_spell_data;

spell_id get_spell_by_name(const char* name);
#endif /* SPELL_DATA_H_ */
