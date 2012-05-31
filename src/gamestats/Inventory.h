#ifndef INVENTORY_H
#define INVENTORY_H

#include "../util/game_basic_structs.h"

const int INVENTORY_SIZE = 40;

struct ItemSlot {
	item_id item;
	int amount;
};

class Inventory {
public:
	Inventory() {
		for (int i = 0; i < INVENTORY_SIZE; i++) {
			inv[i].amount = 0;
		}
	}
	bool add(int item, int number) {

		for (int i = 0; i < INVENTORY_SIZE; i++) {
			if (inv[i].item == item && inv[i].amount > 0) {
				inv[i].item = item;
				inv[i].amount += number;
				return true;
			}
		}
		for (int i = 0; i < INVENTORY_SIZE; i++) {
			if (inv[i].amount == 0) {
				inv[i].item = item;
				inv[i].amount += number;
				return true;
			}
		}
		return false;
	}
	ItemSlot& get(int i){
		return inv[i];
	}
private:
	ItemSlot inv[INVENTORY_SIZE];
};

#endif // INVENTORY_H
