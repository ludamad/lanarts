/*
 * ClassEntry.h:
 *  Represents a class progression
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_api/LuaValueContext.h"

#include <lcommon/strformat.h>

#include "data/game_data.h"
#include "stats/stat_formulas.h"
#include "ClassEntry.h"

sprite_id ClassEntry::get_sprite() {
	return sprites.at(0);
}

const char* ClassEntry::entry_type() {
	return "ClassEntry";
}

static Item parse_as_item(const LuaValueContext& value,
		const char* key = "item") {
	return Item(get_item_by_name(value[key]->as<const char*>()),
			value.defaulted("amount", 1));
}

static Inventory parse_inventory(const LuaValueContext& value) {
	Inventory ret;
	int len = value->objlen();
	for (int i = 1; i <= len; i++) {
		ret.add(parse_as_item(value[i]));
	}
	return ret;
}

static EquipmentStats parse_equipment(const LuaValueContext& value) {
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

static CoreStats parse_core_stats(const LuaValueContext& value) {
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

static AttackStats parse_attack_stats(const LuaValueContext& value) {
	AttackStats ret;

	if (value.has("weapon")) {
		ret.weapon = Weapon(
				get_weapon_by_name(value["weapon"]->as<const char*>()));
	}
	if (value.has("projectile")) {
		ret.projectile = Projectile(
				get_projectile_by_name(value["projectile"]->as<const char*>()));
	}
	return ret;
}

static CombatStats parse_combat_stats(const LuaValueContext& value) {
	CombatStats ret;

	ret.movespeed = value["movespeed"]->as<float>();
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

static ClassSpell parse_class_spell(const LuaValueContext& value) {
	ClassSpell spell;

	spell.spell = get_spell_by_name(value["spell"]->as<const char*>());
	spell.xplevel_required = value["level_needed"]->as<int>();

	return spell;
}

static ClassSpellProgression parse_class_spell_progression(
		const LuaValueContext& value) {
	ClassSpellProgression progression;

	int valuelen = value->objlen();
	for (int i = 1; i <= valuelen; i++) {
		progression.available_spells.push_back(parse_class_spell(value[i]));
	}

	return progression;
}

static void parse_gain_per_level(ClassEntry& entry,
		const LuaValueContext& value) {
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
	ResourceEntryBase::parse_lua_table(table);

	std::string fmt = format("Error while creating ClassEntry: "
			"expected %s%%s field, but did not exist!\n", name.c_str());
	LuaValueContext value(table, fmt.c_str());

	parse_gain_per_level(*this, value["gain_per_level"]);

	spell_progression = parse_class_spell_progression(
			value["available_spells"]);
	starting_stats = parse_combat_stats(value["start_stats"]);

	LuaValueContext sprites = value["sprites"];
	int sprite_len = sprites->objlen();
	for (int i = 1; i <= sprite_len; i++) {
		this->sprites.push_back(res::spriteid(sprites[i]->as<const char*>()));
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
