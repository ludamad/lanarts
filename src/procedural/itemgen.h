/*
 * itemgen.h
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#ifndef ITEMGEN_H_
#define ITEMGEN_H_

#include "mtwist.h"
#include "GeneratedLevel.h"
#include <vector>

class GameState;
//
//struct ItemGenSettings {
//	int item_level;
//	int min_items, max_items;
//	//TODO: Add item probabilities
//	ItemGenSettings(int item_level, int min_items, int max_items) :
//		item_level(item_level), min_items(min_items), max_items(max_items){
//	}
//};

struct ItemGenChance {
	int genchance;//Out of 100%
	int itemtype;
	int groupchance;//Out of 100%
	int groupmin, groupmax;

};
struct ItemGenSettings {
	std::vector<ItemGenChance> item_chances;
	int min_items, max_items;
	ItemGenSettings(std::vector<ItemGenChance>& genchances, int min_items, int max_items) :
		item_chances(genchances), min_items(min_items), max_items(max_items){
	}
};


void generate_items(const ItemGenSettings& is, MTwist& mt, GeneratedLevel& level, GameState* gs);


#endif /* ITEMGEN_H_ */
