/*
 * StoreInventory.cpp:
 * 	Represents the contents of a store.
 */

#include <common/SerializeBuffer.h>

#include "StoreInventory.h"

void StoreInventory::add(const Item & item, int cost) {
	StoreItemSlot newslot(item, cost);

	/* Try to add to new slot */
	for (int i = 0; i < items.size(); i++) {
		if (!slot_filled(i)) {
			items[i] = newslot;
			return;
		}
	}

	items.push_back(newslot);
}

size_t StoreInventory::last_filled_slot() const {
	int i = max_size() - 1;
	for (; i >= 0; i--) {
		if (!items[i].empty()) {
			return i + 1;
		}
	}

	return 0;
}

void StoreInventory::serialize(SerializeBuffer& serializer) {
	serializer.write_container(items);
}

void StoreInventory::deserialize(SerializeBuffer& serializer) {
	serializer.read_container(items);
}

