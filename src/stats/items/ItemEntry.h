/*
 * ItemEntry.h:
 *  Describes an item entry.
 *  This class can represent equipment or consumables such as scrolls etc
 *  It is the base class for various kinds of equipment. Consumables use it directly.
 */

#ifndef ITEMENTRY_H_
#define ITEMENTRY_H_

#include <vector>
#include <algorithm>
#include <string>

#include "lanarts_defines.h"
#include "data/ResourceEntryBase.h"
#include "data/ResourceDataSet.h"

#include "../LuaAction.h"

struct GLimage;
struct SpriteEntry;
namespace ldraw {
class Drawable;
}

class ItemEntry: public ResourceEntryBase {
public:
	virtual const char* entry_type() {
		return item_entry_type.c_str();
	}
	virtual ~ItemEntry() {
	}
	virtual void initialize(lua_State* L) {
		use_action.init(L);
	}

	virtual sprite_id get_sprite() {
		return item_sprite;
	}

	int sell_cost() {
	    return std::max(1, shop_cost.min / 3);
	}
	enum id_type {
		ALWAYS_KNOWN, POTION, SCROLL
	};
	SpriteEntry& item_sprite_entry();
	ldraw::Drawable& item_image();

	LuaLazyValue& inventory_use_func() {
		return use_action.action_func;
	}
	LuaLazyValue& inventory_use_prereq_func() {
		return use_action.prereq_func;
	}

        const std::string& inventory_use_message() {
		return use_action.success_message;
	}
	const std::string& inventory_use_fail_message() {
		return use_action.failure_message;
	}

	virtual void parse_lua_table(const LuaValue& table);

	// Cost when appearing in shops, if (0,0) will not appear in shops.
	Range shop_cost;
	LuaAction use_action;
	LuaValue pickup_call;
	std::string item_entry_type;
	sprite_id item_sprite = 1;
	bool sellable = true;
	bool stackable = true;
};

extern ResourceDataSet<ItemEntry*> game_item_data;

ItemEntry& get_item_entry(item_id id);
item_id get_item_by_name(const char* name, bool error_if_not_found = true);
void clear_item_data(ResourceDataSet<ItemEntry*>& items);

bool is_item_projectile(ItemEntry& ientry);
bool is_item_weapon(ItemEntry& ientry);
bool is_item_equipment(ItemEntry& ientry);

#endif /* ITEMENTRY_H_ */
