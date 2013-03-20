/*
 * SpellEntry.cpp:
 *  Defines the data that goes into a spell
 */

#include <luawrap/luawrap.h>

#include "draw/SpriteEntry.h"

#include "items/ProjectileEntry.h"

#include "SpellEntry.h"

extern std::vector<SpellEntry> game_spell_data;

spell_id get_spell_by_name(const char* name);

//entry.name = parse_str(n["name"]);
//entry.description = parse_defaulted(n, "description", std::string());
//entry.sprite = parse_sprite_number(n, "spr_spell");
//entry.mp_cost = parse_int(n["mp_cost"]);
//entry.cooldown = parse_int(n["cooldown"]);
//if (yaml_has_node(n, "projectile")) {
//	entry.projectile = parse_projectile_name(n["projectile"]);
//}
//entry.action_func = parse_luaexpr(L, n, "action_func");
//entry.autotarget_func = parse_luaexpr(L, n, "autotarget_func", default_autotarget_func);
//entry.prereq_func = parse_luaexpr(L, n, "prereq_func");
//entry.can_cast_with_cooldown = parse_defaulted(n, "can_cast_with_cooldown",
//		false);
//entry.can_cast_with_held_key = parse_defaulted(n, "can_cast_with_held_key",
//			true);
//entry.fallback_to_melee = parse_defaulted(n, "fallback_to_melee",
//			true);

void SpellEntry::parse_lua_table(const LuaValue& table) {
	using namespace luawrap;

	sprite = res::sprite_id(table["spr_spell"].to_str());

	cooldown = table["cooldown"].to_num();
	mp_cost = table["mp_cost"].to_num();

	if (!table["projectile"].isnil()) {
		projectile = Projectile(
				get_projectile_by_name(table["projectile"].to_str()));
	}

	if (table["autotarget_func"].isnil()) {
		table["autotarget_func"] =
				luawrap::globals(table.luastate())["spell_choose_target"];
	}
	autotarget_func = LuaLazyValue(table["autotarget_func"]);
	LANARTS_ASSERT(!autotarget_func.get(table.luastate()).isnil());
	if (!table["action_func"].isnil()) {
		action_func = LuaLazyValue(table["action_func"]);
	}
	if (!table["prereq_func"].isnil()) {
		prereq_func = LuaLazyValue(table["prereq_func"]);
	}

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
	return game_spell_data.at(id);
}
