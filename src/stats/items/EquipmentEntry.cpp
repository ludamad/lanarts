/*
 * EquipmentEntry.cpp:
 *  Represents an equippable item.
 */

#include <cstring>

#include "EquipmentEntry.h"
#include "ProjectileEntry.h"

equipment_id get_equipment_by_name(const char* name) {
	if (strcmp(name, "Nothing") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<EquipmentEntry*>(game_item_data.at(id)));
	return (equipment_id) id;
}

EquipmentEntry& get_equipment_entry(projectile_id id) {
	if (id == NO_ITEM) {
		return get_equipment_entry(get_item_by_name("Nothing"));
	}
	ItemEntry* item = game_item_data.at(id);

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
    } else if (strcmp(name, "amulet") == 0) {
        return EquipmentEntry::AMULET;
    } else if (strcmp(name, "belt") == 0) {
        return EquipmentEntry::BELT;
    } else if (strcmp(name, "legwear") == 0) {
        return EquipmentEntry::LEGWEAR;
	} else if (strcmp(name, "gloves") == 0) {
		return EquipmentEntry::GLOVES;
	} else {
		LANARTS_ASSERT(false);
		return EquipmentEntry::NONE;
	}
}

void EquipmentEntry::parse_lua_table(const LuaValue& table) {
	ItemEntry::parse_lua_table(table);
	if (type == EquipmentEntry::NONE) {
		type = name2type(table["type"].to_str());
	}
	stackable = false;
	use_action = LuaAction(LuaValue());
	stat_modifiers = parse_stat_modifiers(table);
	cooldown_modifiers = parse_cooldown_modifiers(table);
	spells_granted = parse_spells_known(table["spells_granted"]);
}
