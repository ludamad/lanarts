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
#include <vector>

enum {
  ITEM_GOLD = 0,
  ITEM_POTION_HEALTH = 1,
  ITEM_POTION_MANA =  2,
  ITEM_SCROLL_HASTE = 3
};

struct ItemType;

typedef void (*item_actionf)(ItemType* item, GameInst* inst);

struct ItemType {
	const char* name;
	int sprite_number;
	int radius;
	item_actionf action;
	int weapon;
	ItemType(const char* name, int rad, int spriten, item_actionf act, int weapon = -1) :
		name(name), sprite_number(spriten), radius(rad), action(act), weapon(weapon){
	}
};

item_actionf get_action_by_name(const char* name);
int get_item_by_name(const char* name);
extern std::vector<ItemType> game_item_data;



#endif /* ITEM_DATA_H_ */
