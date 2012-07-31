/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>

#include "../lanarts_defines.h"

#include "items.h"

const int INVENTORY_SIZE = 40;

struct ItemSlot {
	Item item;
	int amount;
	ItemSlot() :
			amount(0) {
	}
};

class Inventory {
public:
	Inventory(int size = INVENTORY_SIZE) {
		items.resize(size);
	}
	bool add(const Item& item, int amount);
	ItemSlot& get(int i) {
		return items.at(i);
	}
	bool slot_filled(int i) const {
		return items.at(i).amount > 0;
	}
	size_t max_size() const {
		return items.size();
	}
	size_t last_filled_slot() const;
private:
	std::vector<ItemSlot> items;
};

#endif // INVENTORY_H
