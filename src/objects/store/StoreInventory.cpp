/*
 * StoreInventory.cpp:
 * 	Represents the contents of a store.
 */

#include "StoreInventory.h"

void StoreInventory::add(const Item & item, int amount, int cost) {
	/* Try to add to new slot */
	for (int i = 0; i < items.size(); i++) {
		if (items[i].amount == 0) {
			items[i].item = item;
			items[i].amount = amount;
			return;
		}
	}
	items.push_back(StoreItemSlot(item, amount, cost));
}

size_t StoreInventory::last_filled_slot() const {
	int i = max_size() - 1;
	for (; i >= 0; i--) {
		if (items[i].amount > 0) {
			return i + 1;
		}
	}

	return 0;
}

