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

struct ItemSlot {
	Item item;
	bool equipped;
	bool empty() const {
		return item.empty();
	}
	void clear() {
		item.clear();
		equipped = false;
	}
	int amount() const {
		return item.amount;
	}
	ItemEntry& item_entry() {
		return item.item_entry();
	}
	ItemProperties& properties() {
		return item.properties;
	}
	bool is_same_item(const Item& item) const {
		return this->item.is_same_item(item);
	}
	int id() const {
		return item.id;
	}
	ItemSlot() :
			equipped(false) {
	}
	bool operator==(const ItemSlot& itemslot) const {
		return item == itemslot.item && equipped == itemslot.equipped;
	}
};

class Inventory {
public:
	Inventory(int size = INVENTORY_SIZE) {
		items.resize(size);
	}
	bool add(const Item& item);
	ItemSlot& get(itemslot_t i) {
		return items.at(i);
	}
	bool slot_filled(int i) const {
		return items.at(i).amount() > 0;
	}
	size_t max_size() const {
		return items.size();
	}

	int find_slot(item_id item);
	size_t last_filled_slot() const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
private:
	std::vector<ItemSlot> items;
};

#endif // INVENTORY_H
