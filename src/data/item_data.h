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
#include <string>

#include "../lua/LuaValue.h"

struct ItemEntry {
	const char* name;
	int sprite_number;
	int radius;
	LuaValue effect;
	int weapon;
	ItemEntry(const char* name, int rad, int spriten, const std::string& itemaction, int weapon = -1) :
		name(name), sprite_number(spriten), radius(rad),
		effect(itemaction), weapon(weapon){
	}

	void init(lua_State* L){
		effect.initialize(L);
	}
};

int get_item_by_name(const char* name);
extern std::vector<ItemEntry> game_item_data;



#endif /* ITEM_DATA_H_ */
