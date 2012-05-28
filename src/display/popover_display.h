/*
 * popover_display.h
 *
 *  Created on: May 27, 2012
 *      Author: 100397561
 */

#ifndef POPOVER_DISPLAY_H_
#define POPOVER_DISPLAY_H_

#include "../util/game_basic_structs.h"
#include <vector>

struct ShopEntry {
	item_id item;
	int cost;
};
struct ShopContents {
	std::vector<ShopEntry> items;
};

struct DialogBox {
	std::string content;
};


#endif /* POPOVER_DISPLAY_H_ */
