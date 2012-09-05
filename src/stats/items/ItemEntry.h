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

struct GLimage;
struct SpriteEntry;

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
	SpriteEntry& item_sprite_entry();
	GLimage& item_image();

	LuaValue& inventory_use_func() {
		return use_action.action_func;
	}
	LuaValue& inventory_use_prereq_func() {
		return use_action.prereq_func;
	}
	const std::string& inventory_use_message() {
		return use_action.success_message;
	}
	const std::string& inventory_use_fail_message() {
		return use_action.failure_message;
	}

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

bool is_item_projectile(ItemEntry& ientry);

const char* equip_type_description(ItemEntry& ientry);

#endif /* ITEMENTRY_H_ */
