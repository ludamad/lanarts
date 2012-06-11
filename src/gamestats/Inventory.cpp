/*
 * Inventory.cpp
 *
 *  Created on: Jun 8, 2012
 *      Author: 100397561
 */

#include "Inventory.h"

bool _Inventory::add(const Item& item, int amount) {
	/* Try to merge with existing entry */
	for (int i = 0; i < items.size(); i++) {
		if (items[i].item == item && items[i].amount > 0) {
			items[i].item = item;
			items[i].amount += amount;
			return true;
		}
	}
	/* Try to add to new slot */
	for (int i = 0; i < items.size(); i++) {
		if (items[i].amount == 0) {
			items[i].item = item;
			items[i].amount += amount;
			return true;
		}
	}
	return false;
}
