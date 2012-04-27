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

struct ItemType;

typedef void (*item_actionf)(GameState* gs, ItemType* item, GameInst* inst);

struct ItemType {
	const char* name;
	int sprite_number;
	int radius;
	int action_amount;
	int action_duration;
	item_actionf action;
	int weapon;
	ItemType(const char* name, int rad, int spriten, item_actionf act, int weapon = -1,
			int action_amount = 0, int action_duration = 0) :
		name(name), sprite_number(spriten), radius(rad),
		action_amount(action_amount), action_duration(action_duration), action(act), weapon(weapon){
	}
};

item_actionf get_action_by_name(const char* name);
int get_item_by_name(const char* name);
extern std::vector<ItemType> game_item_data;



#endif /* ITEM_DATA_H_ */
