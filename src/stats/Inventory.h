/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>

#include "../lanarts_defines.h"

#include "items/items.h"

class SerializeBuffer;

const int INVENTORY_SIZE = 40;

typedef int itemslot_t;

class Inventory {
public:
	Inventory(int size = INVENTORY_SIZE) {
		items.resize(size);
	}
	bool add(const Item& item);
	Item& get(itemslot_t i) {
		return items.at(i);
	}
	bool slot_filled(int i) const {
		return items.at(i).amount > 0;
	}
	size_t max_size() const {
		return items.size();
	}

	int find_slot(item_id item);
	size_t last_filled_slot() const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
private:
	std::vector<Item> items;
};

#endif // INVENTORY_H
