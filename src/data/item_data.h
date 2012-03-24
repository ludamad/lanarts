/*
 * item_data.h
 *
 *  Created on: Dec 26, 2011
 *      Author: 100397561
 */

#ifndef ITEM_DATA_H_
#define ITEM_DATA_H_
#include <cstdlib>
#include "../world/objects/GameInst.h"

enum {
  ITEM_GOLD = 0,
  ITEM_POTION = 1
};

typedef void (*item_actionf)(GameInst* inst);

struct ItemType {
	const char* name;
	int sprite_number;
	int radius;
	item_actionf action;
	ItemType(const char* name, int rad, int spriten, item_actionf act) :
		name(name), sprite_number(spriten), radius(rad), action(act){
	}
};

extern ItemType game_item_data[];
extern size_t game_item_n;



#endif /* ITEM_DATA_H_ */
