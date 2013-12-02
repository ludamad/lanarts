/*
 * ProjectileEntry.cpp:
 *  Represents spell/weapon/enemy's projectile data loaded from the yaml
 */

#include <cstring>

#include "ProjectileEntry.h"

projectile_id get_projectile_by_name(const char* name) {
	if (strcmp(name, "Nothing") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<ProjectileEntry*>(game_item_data.at(id)));
	return (projectile_id)id;
}

ProjectileEntry& get_projectile_entry(projectile_id id) {
	if (id == NO_ITEM) {
		return get_projectile_entry(get_item_by_name("Nothing"));
	}
	ItemEntry* item = game_item_data.at(id);

	return dynamic_cast<ProjectileEntry&>(*item);
}
