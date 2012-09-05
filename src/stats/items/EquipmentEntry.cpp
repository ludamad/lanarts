/*
 * ProjectileEntry.cpp:
 *  Represents an equippable item.
 */

#include <cstring>

#include "EquipmentEntry.h"

equipment_id get_equipment_by_name(const char* name) {
	if (strcmp(name, "Nothing") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<EquipmentEntry*>(game_item_data.at(id)));
	return (equipment_id)id;
}

ProjectileEntry& get_projectile_entry(projectile_id id) {
	if (id == NO_ITEM) {
		return get_projectile_entry(get_item_by_name("Nothing"));
	}
	ItemEntry* item = game_item_data.at(id);

	return dynamic_cast<ProjectileEntry&>(*item);
}
