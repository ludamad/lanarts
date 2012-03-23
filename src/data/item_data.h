/*
 * item_data.h
 *
 *  Created on: Dec 26, 2011
 *      Author: 100397561
 */

#ifndef ITEM_DATA_H_
#define ITEM_DATA_H_
#include <cstdlib>

enum {
  ITEM_GOLD = 0,
  ITEM_POTION = 1
};

struct ItemType {
	const char* name;
	int sprite_number;
	int radius;
	ItemType(const char* name, int rad, int spriten) :
		name(name), radius(rad), sprite_number(spriten){
	}
};

extern ItemType game_item_data[];
extern size_t game_item_n;



#endif /* ITEM_DATA_H_ */
