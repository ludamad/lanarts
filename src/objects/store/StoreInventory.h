/*
 * StoreInventory.h:
 * 	Represents the contents of a store.
 */

#ifndef STOREINVENTORY_H_
#define STOREINVENTORY_H_

#include <vector>

#include "../../stats/items.h"

struct StoreItemSlot {
	Item item;
	int amount;
	int cost;
	StoreItemSlot(const Item& item = Item(), int amount = 0, int cost = 0) :
			amount(amount), cost(cost) {
	}
};

class StoreInventory {
public:
	StoreInventory() {
	}
	void add(const Item& item, int amount, int cost);
	StoreItemSlot& get(int i) {
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
	std::vector<StoreItemSlot> items;
};

#endif /* STOREINVENTORY_H_ */