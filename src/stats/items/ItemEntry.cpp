/*
 * ItemEntry.cpp:
 *  Describes an item entry.
 *  This class can represent equipment or consumables such as scrolls etc
 *  It is the base class for various kinds of equipment. Consumables use it directly.
 */

#include <cstdio>
#include <typeinfo>

#include "ItemEntry.h"

std::vector<ItemEntry*> game_item_data;

ItemEntry& get_item_entry(item_id id) {
	return *game_item_data.at(id);
}

void clear_item_data(std::vector<ItemEntry*>& items) {
	for (int i = 0; i < items.size(); i++) {
		delete items[i];
	}
	items.clear();
}

item_id get_item_by_name(const char* name, bool error_if_not_found) {
	for (int i = 0; i < game_item_data.size(); i++) {
		if (name == game_item_data.at(i)->name) {
			return i;
		}
	}
	if (error_if_not_found) {
		/*Error if resource not found*/
		fprintf(stderr, "Failed to load resource!\nname: %s, of type %s\n",
				name, typeid(ItemEntry()).name());
		fflush(stderr);
		LANARTS_ASSERT(false /*resource not found*/);
	}
	return -1;
}
