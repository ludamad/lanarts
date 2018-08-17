/*
 * SpellEntry.cpp:
 *  Defines the data that goes into a spell
 */

#include <luawrap/luawrap.h>

#include "draw/SpriteEntry.h"

#include "data/ResourceDataSet.h"
#include "items/ProjectileEntry.h"

#include "SpellEntry.h"

spell_id get_spell_by_name(const char* name);

void SpellEntry::parse_lua_table(const LuaValue& table) {
	using namespace luawrap;

	sprite = res::sprite_id(table["spr_spell"].to_str());
	LANARTS_ASSERT(sprite != -1);

    cooldown = table["cooldown"].to_num();
    spell_cooldown = defaulted(table, "spell_cooldown", 0);
	mp_cost = table["mp_cost"].to_num();
	cha = table["mp_cost"].to_num();

	if (!table["projectile"].isnil()) {
		projectile = Projectile(
				get_projectile_by_name(table["projectile"].to_str()));
	}

	if (table["autotarget_func"].isnil()) {
		table["autotarget_func"] =
				luawrap::globals(table.luastate())["spell_choose_target"];
	}
	autotarget_func = table["autotarget_func"];
	LANARTS_ASSERT(!autotarget_func.isnil());
    action_func = table["action_func"];
    prereq_func = table["prereq_func"];
    console_draw_func = table["console_draw_func"];

	can_cast_with_cooldown = set_if_nil(table, "can_cast_with_cooldown", false);
	can_cast_with_held_key = set_if_nil(table, "can_cast_with_held_key", true);
	fallback_to_melee = set_if_nil(table, "fallback_to_melee", true);
}

spell_id res::spell_id(const char* name) {
	return get_spell_by_name(name);
}

spell_id res::spell_id(const std::string& name) {
	return spell_id(name.c_str());
}

SpellEntry& res::spell(const char* name) {
	return spell(name);
}

SpellEntry& res::spell(const std::string& name) {
	return spell(name.c_str());
}

SpellEntry& res::spell(::spell_id id) {
	return game_spell_data.get(id);
}
