/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#include "item_data.h"

#include "Inventory.h"

bool Inventory::add(const Item& item, int amount) {
	ItemEntry& ientry = item.item_entry();
	if (ientry.stackable) {
		/* Try to merge with existing entry */
		for (int i = 0; i < items.size(); i++) {
			if (items[i].item == item && items[i].amount > 0) {
				items[i].item = item;
				items[i].amount += amount;
				return true;
			}
		}
	}
	/* Try to add to new slot */
	for (int i = 0; i < items.size(); i++) {
		if (items[i].amount == 0) {
			items[i].item = item;
			items[i].amount += amount;
			return true;
		}
	}
	return false;
}

size_t Inventory::last_filled_slot() const {
	int i = max_size() - 1;
	for (; i >= 0; i--) {
		if (items[i].amount > 0) {
			return i + 1;
		}
	}

	return 0;
}
