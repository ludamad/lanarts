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
	enum equip_type {
		NONE, WEAPON, PROJECTILE, HELMET, SHIELD, ARMOUR, BOOTS
	};
	std::string name;
	int sprite_number;
	int radius;
	LuaValue action_func, prereq_func;
	bool stackable;
	// Extra information if this item is a piece of equipment
	equip_type equipment_type;
	int equipment_id;

	ItemEntry(const std::string& name, int rad, int spriten,
			const std::string& action_luaf, const std::string& prereq_luaf,
			bool stackable, equip_type equipment_type = NONE, int equipment_id =
					-1) :
			name(name), sprite_number(spriten), radius(rad), action_func(
					action_luaf), prereq_func(prereq_luaf), stackable(
					stackable), equipment_type(equipment_type), equipment_id(
					equipment_id) {
	}

	void init(lua_State* L) {
		action_func.initialize(L);
		prereq_func.initialize(L);
	}
};

int get_item_by_name(const char* name);

extern std::vector<ItemEntry> game_item_data;

#endif /* ITEM_DATA_H_ */
