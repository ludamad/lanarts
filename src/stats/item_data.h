/*
 * item_data.h:
 *  Describes an item entry.
 *  This struct is instantiated for every weapon, projectile, and armour entry as well.
 */

#ifndef ITEM_DATA_H_
#define ITEM_DATA_H_
#include <cstdlib>
#include <vector>
#include <string>

#include "../lua/LuaValue.h"

#include "../objects/GameInst.h"

#include "../lanarts_defines.h"
#include "items/ItemEntry.h"
//
//struct ItemEntry {
//	enum equip_type {
//		NONE, WEAPON, PROJECTILE, ARMOUR
//	};
//	enum id_type {
//		ALWAYS_KNOWN, POTION, SCROLL
//	};
//	std::string name, description;
//	std::string use_message;
//	sprite_id sprite;
//	int radius;
//	LuaValue action_func, prereq_func;
//	bool stackable;
//	// Cost when appearing in shops, if (0,0) will not appear in shops.
//	Range shop_cost;
//
//	// Extra information if this item is a piece of equipment
//	equip_type equipment_type;
//	int equipment_id;
//	id_type identify_type;
//
//	ItemEntry(const std::string& name, const std::string& description,
//			const std::string& use_message, int rad, int spriten,
//			const std::string& action_luaf, const std::string& prereq_luaf,
//			bool stackable, const Range& shop_cost = Range(),
//			id_type identify_type = ALWAYS_KNOWN, equip_type equipment_type =
//					NONE, int equipment_id = -1) :
//			name(name), description(description), use_message(use_message), sprite(
//					spriten), radius(rad), action_func(action_luaf), prereq_func(
//					prereq_luaf), stackable(stackable), shop_cost(shop_cost), identify_type(
//					identify_type), equipment_type(equipment_type), equipment_id(
//					equipment_id) {
//	}
//
//	void init(lua_State* L) {
//		action_func.initialize(L);
//		prereq_func.initialize(L);
//	}
//};
//
//const char* equip_type_description(const ItemEntry& ientry);
//
//item_id getItem_by_name(const char* name, bool error_if_not_found = true);
//
//extern std::vector<ItemEntry> _gameItem_data;

#endif /* ITEM_DATA_H_ */
