/*
 * load_stats.cpp:
 *  Routines for loading various combat stats
 */

#include "load_stats.h"

#include "../stat_formulas.h"

AttackStats parse_attack_stats(const YAML::Node & n) {
	std::string name;
	AttackStats ret;

	if (yaml_has_node(n, "weapon")) {
		name = parse_str(n["weapon"]);
		ret.weapon = Weapon(get_weapon_by_name(name.c_str()));
	}
	if (yaml_has_node(n, "projectile")) {
		name = parse_str(n["projectile"]);
		ret.projectile = Projectile(get_projectile_by_name(name.c_str()));
	}
	return ret;
}

static Item parse_as_item(const YAML::Node& n, const char* key = "item") {
	std::string s = parse_str(n[key]);
	int amount = parse_defaulted(n, "amount", 1);
	return Item(get_item_by_name(s.c_str()), amount);
}

Inventory parse_inventory(const YAML::Node& n) {
	Inventory ret;
	for (int i = 0; i < n.size(); i++) {
		const YAML::Node& slot = n[i];
		ret.add(parse_as_item(slot));
	}
	return ret;
}
EquipmentStats parse_equipment(const YAML::Node& n) {
	EquipmentStats ret;
	ret.inventory = parse_inventory(n["inventory"]);
	if (yaml_has_node(n, "weapon")) {
		Item item = parse_as_item(n, "weapon");
		ret.inventory.add(item, true);
	}
	if (yaml_has_node(n, "projectile")) {
		const YAML::Node& pentry = n["projectile"];
		Item item = parse_as_item(pentry);
		ret.inventory.add(item, true);
	}
	return ret;
}

const YAML::Node& operator >>(const YAML::Node& n, CoreStatMultiplier& sm) {
	sm.base = parse_defaulted(n, "base", Range(0, 0));
	sm.strength = parse_defaulted(n, "strength", 0.0f);
	sm.magic = parse_defaulted(n, "magic", 0.0f);
	sm.defence = parse_defaulted(n, "defence", 0.0f);
	sm.willpower = parse_defaulted(n, "willpower", 0.0f);
	return n;
}

CoreStats parse_core_stats(const YAML::Node& n) {
	CoreStats core;
	core.max_mp = parse_defaulted(n, "mp", 0);
	core.max_hp = parse_defaulted(n, "hp", 0);
	core.hpregen = parse_defaulted(n, "hpregen", 0.0);
	core.mpregen = parse_defaulted(n, "mpregen", 0.0);
	core.hp = core.max_hp;
	core.mp = core.max_hp;

	core.strength = parse_defaulted(n, "strength", 0);
	core.defence = parse_defaulted(n, "defence", 0);

	core.magic = parse_defaulted(n, "magic", 0);
	core.willpower = parse_defaulted(n, "willpower", 0);
//	core.physical_reduction = parse_defaulted(n, "reduction", 0);
//	LANARTS_ASSERT(core.physical_reduction == 0);
//	core.magic_reduction = parse_defaulted(n, "magic_reduction", 0);
//	LANARTS_ASSERT(core.magic_reduction == 0);
	return core;
}

CombatStats parse_combat_stats(const YAML::Node& n) {
	CombatStats ret;

	n["movespeed"] >> ret.movespeed;

	if (yaml_has_node(n, "equipment")) {
		ret.equipment = parse_equipment(n["equipment"]);
	}

	ret.core = parse_core_stats(n);

	ret.class_stats.xpneeded = parse_defaulted(n, "xpneeded",
			experience_needed_formula(1));
	ret.class_stats.xplevel = parse_defaulted(n, "xplevel", 1);
	ret.attacks = parse_defaulted(n, "attacks", std::vector<AttackStats>());

	return ret;
}

static void parse_magic_percentage(DamageStats& dm, const YAML::Node& node,
		const char* key) {
	if (yaml_has_node(node, key)) {
		dm.magic_percentage = parse_defaulted(node[key], "magic", 0.0f);
		dm.physical_percentage = parse_defaulted(node[key], "physical", 0.0f);
	} else {
		dm.physical_percentage = 1.0f;
	}
}

DamageStats parse_damage_modifier(const YAML::Node& n) {
	DamageStats dmg;

	dmg.damage_stats = parse_defaulted(n, "damage", CoreStatMultiplier());
	dmg.power_stats = parse_defaulted(n, "power", CoreStatMultiplier());
	parse_magic_percentage(dmg, n, "damage_type");

	dmg.resistability = parse_defaulted(n, "resist_modifier", 1.0f);

	return dmg;
}

ArmourStats parse_defence_modifiers(const YAML::Node& n) {
	ArmourStats def;

	def.damage_reduction = parse_defaulted(n, "reduction",
			CoreStatMultiplier());
	def.resistance = parse_defaulted(n, "resistance", CoreStatMultiplier());
	def.magic_reduction = parse_defaulted(n, "magic_reduction",
			CoreStatMultiplier());
	def.magic_resistance = parse_defaulted(n, "magic_resistance",
			CoreStatMultiplier());

	return def;
}

Attack parse_attack(const YAML::Node& n) {
	Attack atk;
	atk.damage_modifiers = parse_damage_modifier(n);
	atk.cooldown = parse_defaulted(n, "cooldown", 0);
	atk.range = parse_defaulted(n, "range", 0);
	atk.attack_action.action_func = LuaValue(
			parse_defaulted(n, "on_hit_func", std::string()));
	return atk;
}

const YAML::Node& operator >>(const YAML::Node& n,
		std::vector<AttackStats>& attacks) {
	for (int i = 0; i < n.size(); i++) {
		attacks.push_back(parse_attack_stats(n[i]));
	}
	return n;
}

StatModifiers parse_stat_modifiers(const YAML::Node & n) {
	StatModifiers stats;
	if (yaml_has_node(n, "damage_bonuses")) {
		stats.damage_mod = parse_damage_modifier(n["damage_bonuses"]);
	}
	stats.core_mod = parse_defaulted(n, "stat_bonuses", CoreStats());
	stats.armour_mod = parse_defence_modifiers(n);
	return stats;
}

CooldownModifiers parse_cooldown_modifiers(const YAML::Node & n) {
	CooldownModifiers cm;
	cm.melee_cooldown_multiplier = parse_defaulted(n,
			"melee_cooldown_multiplier", 1.0f);
	cm.projectile_cooldown_multiplier = parse_defaulted(n,
			"projectile_cooldown_multiplier", 1.0f);
	cm.rest_cooldown_multiplier = parse_defaulted(n, "rest_cooldown_multiplier",
			1.0f);
	cm.spell_cooldown_multiplier = parse_defaulted(n,
			"spell_cooldown_multiplier", 1.0f);
	return cm;
}

const YAML::Node & operator >>(const YAML::Node & n, CoreStats & core) {
	core = parse_core_stats(n);
	return n;
}
