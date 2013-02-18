/*
 * ClassEntry.h:
 *  Represents a class progression
 */

#include <lua.hpp>

#include "ClassEntry.h"

sprite_id ClassEntry::get_sprite() {
	return sprites.at(0);
}

const char* ClassEntry::entry_type() {
	return "ClassEntry";
}

static Item parse_as_item(const LuaValue& value, const char* key = "item") {
	return Item(get_item_by_name(value.as<const char*>()),
			value["amount"].defaulted(1));
}

static Inventory parse_inventory(const LuaValue& value) {
	Inventory ret;
	int len = value.objlen();
	for (int i = 1; i <= len; i++) {
		ret.add(parse_as_item(value[i]));
	}
	return ret;
}

static EquipmentStats parse_equipment(const LuaValue& value) {
	EquipmentStats ret;
	ret.inventory = parse_inventory(value["inventory"]);

	if (!value["weapon"].isnil()) {
		Item item = parse_as_item(value, "weapon");
		ret.inventory.add(item, true);
	}

	if (!value["projectile"].isnil()) {
		Item item = parse_as_item(value["projectile"]);
		ret.inventory.add(item, true);
	}
	return ret;
}

static CoreStats parse_core_stats(const LuaValue& value) {
	CoreStats core;
	core.max_mp = value["mp"].defaulted(0);
	core.max_hp = value["hp"].defaulted(0);

	core.hp = core.max_hp;
	core.mp = core.max_hp;

	core.hpregen = value["mpregen"].defaulted(0.0f);
	core.mpregen = value["hpregen"].defaulted(0.0f);

	core.strength = value["strength"].defaulted(0);
	core.defence = value["defence"].defaulted(0);

	core.magic = value["magic"].defaulted(0);
	core.willpower = value["willpower"].defaulted(0);
	return core;
}

static AttackStats parse_attack_stats(const LuaValue& value) {
	std::string name;
	AttackStats ret;

	if (!value["weapon"].isnil()) {
		name = value["weapon"];
		ret.weapon = Weapon(
				get_weapon_by_name(value["weapon"].as<const char*>()));
	}
	if (!value["projectile"].isnil()) {
		ret.projectile = Projectile(
				get_projectile_by_name(value["projectile"].as<const char*>()));
	}
	return ret;
}

static CombatStats parse_combat_stats(const LuaValue& value) {
	CombatStats ret;

	ret.movespeed = value["movespeed"];
	if (value["equipment"].isnil()) {
		ret.equipment = parse_equipment(value["equipment"]);
	}

	ret.core = parse_core_stats(value);

	ret.class_stats.xpneeded = value["xpneeded"].defaulted(
			experience_needed_formula(1));
	ret.class_stats.xplevel = value["xplevel"].defaulted(1);

	if (!value["attacks"].isnil()) {
		ret.attacks = parse_attack_stats(value);
	}

	return ret;
}

static ClassSpell parse_class_spell(const LuaStackValue& value) {
	ClassSpell spell;

	spell.spell = get_spell_by_name(value["spell"].as<const char*>());
	spell.xplevel_required = value["level_needed"];

	return spell;
}

static ClassSpellProgression parse_class_spell_progression(
		const LuaStackValue& value) {
	lua_State* L = value.luastate();
	ClassSpellProgression progression;

	int valuelen = lua_objlen(value.luastate(), value.index());
	for (int i = 1; i <= valuelen; i++) {
		lua_rawgeti(L, value.index(), i);
		progression.available_spells.push_back(
				parse_class_spell(LuaStackValue(L, -1)));
	}

	return progression;
}

void ClassEntry::convert_lua() {
	LuaValue& val = lua_table();
	lua_State* L = val.luastate();

	int ltop = lua_gettop(L);

	// Use stack value instead of LuaValue, for performance
	val["start_stats"].push();
	val["sprites"].push();
	val["available_spells"].push();

	// Reference stack values
	LuaStackValue start_stats(L, ltop + 1);
	LuaStackValue sprites(L, ltop + 2);
	LuaStackValue available_spells(L, ltop + 3);

	on_level_gain = val["on_level_gain"];

	spell_progression = parse_class_spell_progression(available_spells);
	starting_stats = parse_combat_stats(start_stats);

	int sprite_len = lua_objlen(L, ltop + 2);
	for (int i = 1; i <= sprite_len; i++) {
		lua_rawgeti(L, ltop + 2, i);
		this->sprites.push_back(res::spriteid(lua_tostring(L, -1)));
		lua_pop(L, 1);
	}

	// Clean up stack values
	lua_pop(L, 3);
}
