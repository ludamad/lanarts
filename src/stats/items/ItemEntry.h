/*
 * ItemEntry.h:
 *  Describes an item entry.
 *  This class can represent equipment or consumables such as scrolls etc
 *  It is the base class for various kinds of equipment. Consumables use it directly.
 */

#ifndef ITEMENTRY_H_
#define ITEMENTRY_H_

#include <vector>
#include <string>

#include "../../lanarts_defines.h"
#include "../../data/BaseDataEntry.h"

#include "../LuaAction.h"

class ItemEntry: public BaseDataEntry {
public:
	ItemEntry() :
			item_sprite(-1), stackable(true) {
	}
	virtual ~ItemEntry() {
	}
	virtual void init(lua_State* L) {
		use_action.init(L);
	}

	enum id_type {
		ALWAYS_KNOWN, POTION, SCROLL
	};
	// Cost when appearing in shops, if (0,0) will not appear in shops.
	Range shop_cost;
	LuaAction use_action;
	sprite_id item_sprite;
	bool stackable;
};

extern std::vector<ItemEntry*> game_item_data;

ItemEntry& get_item_entry(item_id id);
item_id get_item_by_name(const char* name, bool error_if_not_found = true);
void clear_item_data(std::vector<ItemEntry*>& items);

#endif /* ITEMENTRY_H_ */
