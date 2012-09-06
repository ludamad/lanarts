/*
 * StoreInventory.h:
 * 	Represents the contents of a store.
 */

#ifndef STOREINVENTORY_H_
#define STOREINVENTORY_H_

#include <vector>

#include "../../stats/items/items.h"

struct StoreItemSlot {
	Item item;
	int cost;
	StoreItemSlot(const Item& item = Item(), int cost = 0) :
			item(item), cost(cost) {
	}

	ItemEntry& item_entry () {
		return item.item_entry();
	}
	bool empty() const {
		return item.empty();
	}
	void clear() {
		item.clear();
		cost = 0;
	}
};

class SerializeBuffer;

class StoreInventory {
public:
	StoreInventory(int maxsize = 40) :
			items(maxsize) {
	}
	void add(const Item& item, int cost);
	StoreItemSlot& get(int i) {
		return items.at(i);
	}
	bool slot_filled(int i) const {
		return !items.at(i).item.empty();
	}
	size_t max_size() const {
		return items.size();
	}
	size_t last_filled_slot() const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
private:
	std::vector<StoreItemSlot> items;
};

#endif /* STOREINVENTORY_H_ */
