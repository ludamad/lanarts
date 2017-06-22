/*
 * SpellEntry.h:
 *  Defines the data that goes into a spell
 */

#ifndef SPELLENTRY_H_
#define SPELLENTRY_H_

#include <string>
#include <vector>

#include "data/ResourceEntryBase.h"
#include "data/ResourceDataSet.h"

#include <lcommon/LuaLazyValue.h>

#include "lanarts_defines.h"

#include "items/items.h"

struct SpellEntry: public ResourceEntryBase {
	sprite_id sprite = NONE;
	int mp_cost = 0, cooldown = 0, spell_cooldown = 0;
	LuaValue action_func; // Immediate action
    LuaValue console_draw_func; // Drawing in the help bar
    LuaValue autotarget_func; // Auto-target func
    LuaValue prereq_func; // Pre-req to casting

    Projectile projectile;
	bool can_cast_with_cooldown = false, can_cast_with_held_key = false, fallback_to_melee = false;

	virtual const char* entry_type() {
		return "Spell";
	}
	virtual sprite_id get_sprite() {
		return sprite;
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

extern ResourceDataSet<SpellEntry> game_spell_data;
#endif /* SPELLENTRY_H_ */
