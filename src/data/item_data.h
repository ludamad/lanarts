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

#include "../util/LuaValue.h"

struct ItemEntry {
	const char* name;
	int sprite_number;
	int radius;
	LuaValue action_func, prereq_func;
	bool stackable;
	int weapon;
	ItemEntry(const char* name, int rad, int spriten,
			const std::string& action_luaf, const std::string& prereq_luaf,
			bool stackable, int weapon = -1) :
		name(name), sprite_number(spriten), radius(rad),
		action_func(action_luaf), prereq_func(prereq_luaf),
		stackable(stackable), weapon(weapon){
	}

	void init(lua_State* L){
		action_func.initialize(L);
		prereq_func.initialize(L);
	}
};

int get_item_by_name(const char* name);
extern std::vector<ItemEntry> game_item_data;



#endif /* ITEM_DATA_H_ */
