/*
 * itemgen_data.h:
 *  Defines item generation chances and quantities, split up into various stages of the game
 */

#ifndef ITEMGEN_DATA_H_
#define ITEMGEN_DATA_H_

#include <string>
#include <vector>

#include "../lanarts_defines.h"

struct ItemGenChance {
	int genchance; //Out of 100%
	item_id itemtype;
	Range quantity;
};

struct ItemGenList {
	std::string name;
	std::vector<ItemGenChance> items;
};

itemgenlist_id get_itemgenlist_by_name(const char* name, bool error_if_not_found = true);

extern std::vector<ItemGenList> game_itemgenlist_data;

#endif /* ITEMGEN_DATA_H_ */
