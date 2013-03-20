/*
 * ClassEntry.h:
 *  Represents a class progression
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include <lcommon/strformat.h>

#include "data/game_data.h"
#include "stats/stat_formulas.h"

#include "SpellEntry.h"
#include "ClassEntry.h"

sprite_id ClassEntry::get_sprite() {
	return sprites.at(0);
}

const char* ClassEntry::entry_type() {
	return "Class";
}

static Item parse_as_item(const LuaField& value,
		const char* key = "item") {
	return Item(get_item_by_name(value[key].to_str()),
			value.defaulted("amount", 1));
}

static Inventory parse_inventory(const LuaField& value) {
	Inventory ret;
	int len = value.objlen();
	for (int i = 1; i <= len; i++) {
		ret.add(parse_as_item(value[i]));
	}
	return ret;
}

static EquipmentStats parse_equipment(const LuaField& value) {
	EquipmentStats ret;
	if (value.has("inventory")) {
		ret.inventory = parse_inventory(value["inventory"]);
	}

	if (value.has("weapon")) {
		Item item = parse_as_item(value, "weapon");
		ret.inventory.add(item, true);
	}

	if (value.has("projectile")) {
		Item item = parse_as_item(value["projectile"]);
		ret.inventory.add(item, true);
	}
	return ret;
}

static CoreStats parse_core_stats(const LuaField& value) {
	CoreStats core;
	core.max_mp = value.defaulted("mp", 0);
	core.max_hp = value.defaulted("hp", 0);

	core.hp = core.max_hp;
	core.mp = core.max_mp;

	core.hpregen = value.defaulted("mpregen", 0.0f);
	core.mpregen = value.defaulted("hpregen", 0.0f);

	core.strength = value.defaulted("strength", 0);
	core.defence = value.defaulted("defence", 0);

	core.magic = value.defaulted("magic", 0);
	core.willpower = value.defaulted("willpower", 0);
	return core;
}

static AttackStats parse_attack_stats(const LuaField& value) {
	AttackStats ret;

	if (value.has("weapon")) {
		ret.weapon = Weapon(
				get_weapon_by_name(value["weapon"].to_str()));
	}
	if (value.has("projectile")) {
		ret.projectile = Projectile(
				get_projectile_by_name(value["projectile"].to_str()));
	}
	return ret;
}

static CombatStats parse_combat_stats(const LuaField& value) {
	CombatStats ret;

	ret.movespeed = value["movespeed"].to_num();
	if (value.has("equipment")) {
		ret.equipment = parse_equipment(value["equipment"]);
	}

	ret.core = parse_core_stats(value);

	ret.class_stats.xpneeded = value.defaulted("xpneeded",
			experience_needed_formula(1));
	ret.class_stats.xplevel = value.defaulted("xplevel", 1);

	if (value.has("attacks")) {
		ret.attacks.push_back(parse_attack_stats(value));
	}

	return ret;
}

static ClassSpell parse_class_spell(const LuaField& value) {
	ClassSpell spell;

	spell.spell = res::spell_id(value["spell"].to_str());
	spell.xplevel_required = value["level_needed"].to_int();

	return spell;
}

static ClassSpellProgression parse_class_spell_progression(
		const LuaField& value) {
	ClassSpellProgression progression;

	int valuelen = value.objlen();
	for (int i = 1; i <= valuelen; i++) {
		progression.available_spells.push_back(parse_class_spell(value[i]));
	}

	return progression;
}

static void parse_gain_per_level(ClassEntry& entry,
		const LuaField& value) {
	entry.hp_perlevel = value.defaulted("hp", 0);
	entry.mp_perlevel = value.defaulted("mp", 0);

	entry.str_perlevel = value.defaulted("strength", 0);
	entry.def_perlevel = value.defaulted("defence", 0);
	entry.mag_perlevel = value.defaulted("magic", 0);
	entry.will_perlevel = value.defaulted("willpower", 0);

	entry.mpregen_perlevel = value.defaulted("mpregen", 0.0f);
	entry.hpregen_perlevel = value.defaulted("hpregen", 0.0f);
}

void ClassEntry::parse_lua_table(const LuaValue& table) {
	parse_gain_per_level(*this, table["gain_per_level"]);

	spell_progression = parse_class_spell_progression(
			table["available_spells"]);
	starting_stats = parse_combat_stats(table["start_stats"]);

	LuaField sprites = table["sprites"];
	int sprite_len = sprites.objlen();
	for (int i = 1; i <= sprite_len; i++) {
		this->sprites.push_back(res::sprite_id(sprites[i].to_str()));
	}
}
namespace res {
	::class_id class_id(const char* name) {
		return get_class_by_name(name);
	}

	::class_id class_id(const std::string& name) {
		return get_class_by_name(name.c_str());
	}

	ClassEntry class_entry(const char* name) {
		return game_class_data.at(class_id(name));
	}

	ClassEntry class_entry(const std::string& name) {
		return game_class_data.at(class_id(name));
	}

	ClassEntry class_entry(::class_id id) {
		return game_class_data.at(id);
	}
}
