/*
 * EquipmentEntry.cpp:
 *  Represents an equippable item.
 */

#include <cstring>

#include <luawrap/luawrap.h>

#include "EquipmentEntry.h"
#include "stats/effect_data.h"
#include "ProjectileEntry.h"
#include <vector>
#include <string>

using namespace std;

equipment_id get_equipment_by_name(const char* name) {
	if (strcmp(name, "Nothing") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<EquipmentEntry*>(game_item_data.get(id)));
	return (equipment_id) id;
}

EquipmentEntry& get_equipment_entry(projectile_id id) {
	if (id == NO_ITEM) {
		return get_equipment_entry(get_item_by_name("Nothing"));
	}
	ItemEntry* item = game_item_data.get(id);

	return dynamic_cast<EquipmentEntry&>(*item);
}

const char* EquipmentEntry::entry_type() {
	switch (type) {
	case EquipmentEntry::BODY_ARMOUR:
		return "Armour";
	case EquipmentEntry::BOOTS:
		return "Boots";
    case EquipmentEntry::HEADGEAR:
        return "Headgear";
    case EquipmentEntry::LANART:
        return "Lanart";
    case EquipmentEntry::AMULET:
        return "Amulet";
    case EquipmentEntry::LEGWEAR:
        return "Legwear";
    case EquipmentEntry::BELT:
        return "Belt";
	case EquipmentEntry::GLOVES:
		return "Gloves";
	case EquipmentEntry::RING:
		return "Ring";
	case EquipmentEntry::WEAPON:
		return "Weapon";
	case EquipmentEntry::AMMO: {
		ProjectileEntry* pentry = dynamic_cast<ProjectileEntry*>(this);
		if (pentry->is_standalone()) {
			return "Throwing Weapon";
		} else {
			return "Ammunition";
		}
	}
	case EquipmentEntry::NONE:
		return "One-time Use";
	}
	return "";
}

static EquipmentEntry::equip_type name2type(const char* name) {
	if (strcmp(name, "armour") == 0) {
		return EquipmentEntry::BODY_ARMOUR;
	} else if (strcmp(name, "ring") == 0) {
		return EquipmentEntry::RING;
	} else if (strcmp(name, "boots") == 0) {
		return EquipmentEntry::BOOTS;
    } else if (strcmp(name, "helmet") == 0) {
        return EquipmentEntry::HEADGEAR;
    } else if (strcmp(name, "lanart") == 0) {
        return EquipmentEntry::LANART;
    } else if (strcmp(name, "amulet") == 0) {
        return EquipmentEntry::AMULET;
    } else if (strcmp(name, "belt") == 0) {
        return EquipmentEntry::BELT;
    } else if (strcmp(name, "projectile") == 0) {
        return EquipmentEntry::AMMO;
    } else if (strcmp(name, "legwear") == 0) {
        return EquipmentEntry::LEGWEAR;
	} else if (strcmp(name, "gloves") == 0) {
		return EquipmentEntry::GLOVES;
	} else {
		LANARTS_ASSERT(false);
		return EquipmentEntry::NONE;
	}
}

// Defined in objects/load_enemy_data.cpp
std::vector<StatusEffect> load_statuses(const LuaValue& effects);

void EquipmentEntry::parse_lua_table(const LuaValue& table) {
	ItemEntry::parse_lua_table(table);
	if (type == EquipmentEntry::NONE) {
		type = name2type(table["type"].to_str());
	}
	stackable = false;
	use_action = LuaAction();
	stat_modifiers = parse_stat_modifiers(table);
	CooldownModifiers cdown;
	if (!table["stat_bonuses"].isnil()) {
            cdown = parse_cooldown_modifiers(table["stat_bonuses"]);
	}
	if (cdown.is_default()) {
		// Fallback behaviour, only if cooldown modifiers not shown in stat_bonuses:
        cdown = parse_cooldown_modifiers(table);
	}
	cooldown_modifiers = cdown;
	spells_granted = parse_spells_known(table["spells_granted"]);
    effect_modifiers.status_effects = load_statuses(table["effects_granted"]);
	auto_equip = luawrap::defaulted(table, "auto_equip", true);
	if (table["auto_equip"].isnil()) {
		// Set autoequip to false if there are negative attributes:
		bool has_bad = (cooldown_modifiers.ranged_cooldown_multiplier > 1) ||
			(cooldown_modifiers.melee_cooldown_multiplier > 1) ||
			(cooldown_modifiers.spell_cooldown_multiplier > 1) ||
			(cooldown_modifiers.rest_cooldown_multiplier > 1) ||
			(stat_modifiers.armour_mod.magic_resistance.base.min < 0) ||
			(stat_modifiers.armour_mod.resistance.base.min < 0) ||
			(stat_modifiers.damage_mod.damage_stats.base.min < 0) ||
			(stat_modifiers.damage_mod.power_stats.base.min < 0) ||
			(stat_modifiers.core_mod.max_hp < 0) ||
			(stat_modifiers.core_mod.hp < 0) ||
			(stat_modifiers.core_mod.max_mp < 0) ||
			(stat_modifiers.core_mod.mp < 0) ||
			(stat_modifiers.core_mod.hpregen < 0) ||
			(stat_modifiers.core_mod.magic < 0) ||
			(stat_modifiers.core_mod.mpregen < 0) ||
			(stat_modifiers.core_mod.spell_velocity_multiplier < 1) ||
			(stat_modifiers.core_mod.strength < 0) ||
			(stat_modifiers.core_mod.willpower < 0) ||
			(stat_modifiers.core_mod.defence < 0);
		// auto equip if there are no downsides
		auto_equip = !has_bad;
	}
}
