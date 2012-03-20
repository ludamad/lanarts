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

struct ItemGenSettings {
	int item_level;
	int min_items, max_items;
	//TODO: Add item probabilities
	ItemGenSettings(int item_level, int min_items, int max_items) :
		item_level(item_level), min_items(min_items), max_items(max_items){
	}
};


void generate_items(const ItemGenSettings& rs, MTwist& mt, GeneratedLevel& room);


#endif /* ITEMGEN_H_ */
