/*
 * ClassEntry.h:
 *  Represents a class progression
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "data/game_data.h"
#include "stats/stat_formulas.h"
#include "ClassEntry.h"

sprite_id ClassEntry::get_sprite() {
	return sprites.at(0);
}

const char* ClassEntry::entry_type() {
	return "ClassEntry";
}

static Item parse_as_item(const LuaValue& value, const char* key = "item") {
	return Item(get_item_by_name(value[key].as<const char*>()),
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
	if (!value["inventory"].isnil()) {
		ret.inventory = parse_inventory(value["inventory"]);
	}

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
	core.mp = core.max_mp;

	core.hpregen = value["mpregen"].defaulted(0.0f);
	core.mpregen = value["hpregen"].defaulted(0.0f);

	core.strength = value["strength"].defaulted(0);
	core.defence = value["defence"].defaulted(0);

	core.magic = value["magic"].defaulted(0);
	core.willpower = value["willpower"].defaulted(0);
	return core;
}

static AttackStats parse_attack_stats(const LuaValue& value) {
	AttackStats ret;

	if (!value["weapon"].isnil()) {
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
	if (!value["equipment"].isnil()) {
		ret.equipment = parse_equipment(value["equipment"]);
	}

	ret.core = parse_core_stats(value);

	ret.class_stats.xpneeded = value["xpneeded"].defaulted(
			experience_needed_formula(1));
	ret.class_stats.xplevel = value["xplevel"].defaulted(1);

	if (!value["attacks"].isnil()) {
		ret.attacks.push_back(parse_attack_stats(value));
	}

	return ret;
}

static ClassSpell parse_class_spell(const LuaValue& value) {
	ClassSpell spell;

	spell.spell = get_spell_by_name(value["spell"].as<const char*>());
	spell.xplevel_required = value["level_needed"];

	return spell;
}

static ClassSpellProgression parse_class_spell_progression(
		const LuaValue& value) {
	ClassSpellProgression progression;

	int valuelen = value.objlen();
	for (int i = 1; i <= valuelen; i++) {
		progression.available_spells.push_back(parse_class_spell(value[i]));
	}

	return progression;
}

static void parse_gain_per_level(ClassEntry& entry, const LuaValue& value) {
	entry.hp_perlevel = value["hp"].defaulted(0);
	entry.mp_perlevel = value["mp"].defaulted(0);

	entry.str_perlevel = value["strength"].defaulted(0);
	entry.def_perlevel = value["defence"].defaulted(0);
	entry.mag_perlevel = value["magic"].defaulted(0);
	entry.will_perlevel = value["willpower"].defaulted(0);

	entry.mpregen_perlevel = value["mpregen"].defaulted(0.0f);
	entry.hpregen_perlevel = value["hpregen"].defaulted(0.0f);
}

void ClassEntry::parse_lua_table(const LuaValue& table) {
	ResourceEntryBase::parse_lua_table(table);

	parse_gain_per_level(*this, table["gain_per_level"]);

	spell_progression = parse_class_spell_progression(
			table["available_spells"]);
	starting_stats = parse_combat_stats(table["start_stats"]);

	LuaValue sprites = table["sprites"];
	int sprite_len = sprites.objlen();
	for (int i = 1; i <= sprite_len; i++) {
		this->sprites.push_back(res::spriteid(sprites[i].as<const char*>()));
	}
}
namespace res {
	class_id classid(const char* name) {
		return get_class_by_name(name);
	}

	class_id classid(const std::string& name) {
		return get_class_by_name(name.c_str());
	}

	ClassEntry class_entry(const char* name) {
		return game_class_data.at(classid(name));
	}

	ClassEntry class_entry(const std::string& name) {
		return game_class_data.at(classid(name));
	}

	ClassEntry class_entry(class_id id) {
		return game_class_data.at(id);
	}
}
