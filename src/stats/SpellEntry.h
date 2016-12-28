/*
 * SpellEntry.h:
 *  Defines the data that goes into a spell
 */

#ifndef SPELLENTRY_H_
#define SPELLENTRY_H_

#include <string>
#include <vector>

#include "data/ResourceEntryBase.h"

#include <lcommon/LuaLazyValue.h>

#include "lanarts_defines.h"

#include "items/items.h"

struct SpellEntry: public ResourceEntryBase {
	sprite_id sprite = NONE;
	int mp_cost = 0, cooldown = 0, spell_cooldown = 0;
	LuaLazyValue action_func; // Immediate action
	LuaLazyValue autotarget_func; // Auto-target func
	LuaLazyValue prereq_func; // Pre-req to casting
	Projectile projectile; // Projectile used, if any
	bool can_cast_with_cooldown = false, can_cast_with_held_key = false, fallback_to_melee = false;

	virtual const char* entry_type() {
		return "Spell";
	}
	virtual sprite_id get_sprite() {
		return sprite;
	}

	void initialize(lua_State* L) {
		action_func.initialize(L);
		autotarget_func.initialize(L);
		prereq_func.initialize(L);
	}
	bool uses_projectile() {
		return projectile.id != NO_ITEM;
	}

	virtual void parse_lua_table(const LuaValue& table);
};

namespace res {
	::spell_id spell_id(const char* name);
	::spell_id spell_id(const std::string& name);
	SpellEntry& spell(const char* name);
	SpellEntry& spell(const std::string& name);
	SpellEntry& spell(::spell_id id);
}
#endif /* SPELLENTRY_H_ */
