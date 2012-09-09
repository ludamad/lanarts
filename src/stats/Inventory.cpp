/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#include "../serialize/SerializeBuffer.h"

#include "items/ItemEntry.h"

#include "Inventory.h"

bool Inventory::add(const Item& item) {
	ItemEntry& ientry = item.item_entry();
	if (ientry.stackable) {
		/* Try to merge with existing entry */
		for (int i = 0; i < items.size(); i++) {
			if (items[i].is_same_item(item) && !items[i].empty()) {
				items[i].item.amount += item.amount;
				return true;
			}
		}
	}
	/* Try to add to new slot */
	for (int i = 0; i < items.size(); i++) {
		if (items[i].empty()) {
			items[i].item = item;
			items[i].equipped = false;
			return true;
		}
	}
	for (int i = 0; i < items.size(); i++) {
		if (items[i].amount() > 0) {
			LANARTS_ASSERT(
					items[i].amount() == 1 || items[i].item_entry().stackable);
		}
	}
	return false;
}

int Inventory::find_slot(item_id item) {
	for (int i = 0; i < max_size(); i++) {
		if (get(i).id() == item)
			return i;
	}
	return -1;
}

size_t Inventory::last_filled_slot() const {
	int i = max_size() - 1;
	for (; i >= 0; i--) {
		if (items[i].amount() > 0) {
			return i + 1;
		}
	}

	return 0;
}

void Inventory::serialize(SerializeBuffer& serializer) {
	serializer.write_int(items.size());
	for (int i = 0; i < items.size(); i++) {
		items[i].item.serialize(serializer);
		serializer.write_byte(items[i].equipped);
	}
}

void Inventory::deserialize(SerializeBuffer& serializer) {
	int size;
	serializer.read_int(size);
	items.resize(size);
	for (int i = 0; i < items.size(); i++) {
		items[i].item.deserialize(serializer);
		serializer.read_byte(items[i].equipped);
	}
}

