/*
 * load_stats.h:
 *  Routines for loading various combat stats
 */

#ifndef LOAD_STATS_H_
#define LOAD_STATS_H_

#include "../../data/yaml_util.h"

#include "../items/EquipmentEntry.h"
#include "../items/ItemEntry.h"

#include "../Attack.h"
#include "../combat_stats.h"
#include "../stat_modifiers.h"

// In load_item_data.cpp:
void parse_item_entry(const YAML::Node& n, ItemEntry& entry);

// In load_armour_data.cpp:
void parse_equipment_entry(const YAML::Node& n, EquipmentEntry& entry);

Attack parse_attack(const YAML::Node& n);

CoreStats parse_core_stats(const YAML::Node& n);
AttackStats parse_attack_stats(const YAML::Node& n);
CombatStats parse_combat_stats(const YAML::Node& n);

DamageStats parse_damage_modifier(const YAML::Node& n);
StatModifiers parse_stat_modifiers(const YAML::Node& n);
CooldownModifiers parse_cooldown_modifiers(const YAML::Node& n);

const YAML::Node& operator >>(const YAML::Node& n,
		std::vector<AttackStats>& attacks);
const YAML::Node& operator >>(const YAML::Node& n, CoreStatMultiplier& sm);
const YAML::Node& operator >>(const YAML::Node& n, CoreStats& core);

#endif /* LOAD_STATS_H_ */
