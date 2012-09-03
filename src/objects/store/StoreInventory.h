/*
 * StoreInventory.h:
 * 	Represents the contents of a store.
 */

#ifndef STOREINVENTORY_H_
#define STOREINVENTORY_H_

#include <vector>

#include "../../stats/items.h"

struct StoreItemSlot {
	_Item item;
	int amount, cost;
	StoreItemSlot(const _Item& item = _Item(), int amount = 0, int cost = 0) :
			item(item), amount(amount), cost(cost) {
	}
};

class SerializeBuffer;

class StoreInventory {
public:
	StoreInventory(int maxsize = 40) :
			items(maxsize) {
	}
	void add(const _Item& item, int amount, int cost);
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

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
private:
	std::vector<StoreItemSlot> items;
};

#endif /* STOREINVENTORY_H_ */
