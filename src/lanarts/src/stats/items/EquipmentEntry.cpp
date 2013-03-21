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
	return (equipment_id)id;
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

