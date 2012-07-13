/*
 * spell_data.h:
 *  Defines the data that goes into a spell
 */

#ifndef SPELL_DATA_H_
#define SPELL_DATA_H_

#include <string>
#include <vector>

#include "../gamestats/items.h"

#include "../util/game_basic_structs.h"
#include "../util/LuaValue.h"

struct SpellEntry {
	std::string name, description;
	sprite_id sprite;
	int mp_cost;
	LuaValue action; //Immediate action
	Projectile projectile; //Projectile used, if any
	bool can_cast_without_cooldown;
	SpellEntry() :
			sprite(-1), mp_cost(0), can_cast_without_cooldown(false) {
	}

	void init(lua_State* L) {
		action.initialize(L);
	}
	bool uses_projectile() {
		return projectile.valid_projectile();
	}
};

extern std::vector<SpellEntry> game_spell_data;

spell_id get_spell_by_name(const char* name);
#endif /* SPELL_DATA_H_ */
