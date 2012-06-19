/*
 * itemgen.h
 *  Defines parameters for item generation as well as the generate_items function
 */

#ifndef ITEMGEN_H_
#define ITEMGEN_H_

#include "../util/mtwist.h"
#include "GeneratedLevel.h"
#include <vector>

class GameState;

struct ItemGenChance {
	int genchance;//Out of 100%
	int itemtype;
	Range quantity;
};
struct ItemGenSettings {
	std::vector<ItemGenChance> item_chances;
	Range num_items;
};


void generate_items(const ItemGenSettings& is, MTwist& mt, GeneratedLevel& level, GameState* gs);


#endif /* ITEMGEN_H_ */
