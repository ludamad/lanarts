/*
 * SpellEntry.h:
 *  Defines the data that goes into a spell
 */

#ifndef SPELLENTRY_H_
#define SPELLENTRY_H_

#include <string>
#include <vector>

#include "../data/BaseDataEntry.h"

#include <common/lua/LuaValue.h>

#include "../lua/luaexpr.h"
#include "../lanarts_defines.h"

#include "items/items.h"

struct SpellEntry: public BaseDataEntry {
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
	virtual const char* entry_type() {
		return "Spell";
	}
	virtual sprite_id get_sprite() {
		return sprite;
	}

	void init(lua_State* L) {
		luavalue_call_and_store(L, action_func);
		luavalue_call_and_store(L, autotarget_func);
		luavalue_call_and_store(L, prereq_func);
	}
	bool uses_projectile() {
		return projectile.id != NO_ITEM;
	}
};

extern std::vector<SpellEntry> game_spell_data;

spell_id get_spell_by_name(const char* name);
#endif /* SPELLENTRY_H_ */
