/*
 * WeaponEntry.cpp:
 *  Represents enemy & player weapon (ie, for enemies, natural attacks) data
 *  loaded from the yaml
 */

#include <cstring>

#include "WeaponEntry.h"

weapon_id get_weapon_by_name(const char* name) {
	if (strcmp(name, "Unarmed") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<WeaponEntry*>(game_item_data.at(id)));
	return (weapon_id)id;
}

WeaponEntry& get_weapon_entry(weapon_id id) {
	if (id == NO_ITEM) {
		return get_weapon_entry(get_item_by_name("Unarmed"));
	}
	ItemEntry* item = game_item_data.at(id);

	return dynamic_cast<WeaponEntry&>(*item);
}
