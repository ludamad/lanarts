/*
 * Itemcpp:
 *  Describes an item
 *  This class can represent equipment or consumables such as scrolls etc
 *  It is the base class for various kinds of equipment. Consumables use it directly.
 */

#include <cstdio>
#include <typeinfo>

#include <luawrap/luawrap.h>
#include <ldraw/DrawOptions.h>
#include <data/lua_util.h>

#include "draw/SpriteEntry.h"

#include "ItemEntry.h"
#include "EquipmentEntry.h"
#include "WeaponEntry.h"
#include "ProjectileEntry.h"

SpriteEntry& ItemEntry::item_sprite_entry() {
	return game_sprite_data.get(item_sprite);
}
ldraw::Drawable& ItemEntry::item_image() {
	return res::sprite(item_sprite);
}

ResourceDataSet<ItemEntry*> game_item_data;

ItemEntry& get_item_entry(item_id id) {
	return *game_item_data.get(id);
}

void clear_item_data(ResourceDataSet<ItemEntry*>& items) {
	for (int i = 0; i < items.size(); i++) {
            delete items.get(i);
	}
	items.clear();
}

item_id get_item_by_name(const char* name, bool error_if_not_found) {
	for (int i = 0; i < game_item_data.size(); i++) {
		if (name == game_item_data.get(i)->name) {
			return i;
		}
	}
	if (error_if_not_found) {
		/*Error if resource not found*/
		fprintf(stderr, "Failed to load resource!\nname: %s, of type %s\n",
				name, typeid(ItemEntry()).name());
		fflush(stderr);
		LANARTS_ASSERT(false /*resource not found*/);
	}
	return -1;
}

bool is_item_projectile(ItemEntry & ientry) {
	return dynamic_cast<ProjectileEntry*>(&ientry) != NULL;
}

bool is_item_weapon(ItemEntry & ientry) {
	return dynamic_cast<WeaponEntry*>(&ientry) != NULL;
}

bool is_item_equipment(ItemEntry & ientry) {
	return dynamic_cast<EquipmentEntry*>(&ientry) != NULL;
}

void ItemEntry::parse_lua_table(const LuaValue& table) {
	using namespace luawrap;
	ResourceEntryBase::parse_lua_table(table);

	shop_cost = defaulted(table, "shop_cost", Range());
	sellable = defaulted(table, "sellable", true);

	use_action.success_message = set_if_nil(table, "use_message",
			std::string());
	use_action.failure_message = set_if_nil(table, "cant_use_message",
			std::string());
        pickup_call = table["pickup_func"];

	if (!table["action_func"].isnil()) {
		use_action.action_func = table["action_func"];
	}
	if (!table["prereq_func"].isnil()) {
		use_action.prereq_func = table["prereq_func"];
	}
	console_draw_func = table["console_draw_func"];
	item_draw_func = table["item_draw_func"];
	item_sprite = -1;
	if (!table["spr_item"].isnil()) {
		item_sprite = (sprite_id)game_sprite_data.get_id(table["spr_item"].to_str());
        if (item_sprite == -1) {
            printf("Expected a valid 'spr_item' field on item definition! Got '%s'.\n",
                   table["spr_item"].to_str());
            exit(1);
        }
	}
	item_entry_type = set_if_nil(table, "entry_type", (const char*)"One-time Use");

	stackable = set_if_nil(table, "stackable", true);

}

void ItemEntry::draw(const ldraw::DrawOptions& options, Pos xy) {
	auto& sprite = item_sprite_entry();
	sprite.sprite.draw(options, xy);
	lcall(item_draw_func, raw, options, xy.x, xy.y);
}