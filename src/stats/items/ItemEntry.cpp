/*
 * ItemEntry.cpp:
 *  Describes an item entry.
 *  This class can represent equipment or consumables such as scrolls etc
 *  It is the base class for various kinds of equipment. Consumables use it directly.
 */

#include <cstdio>
#include <typeinfo>

#include "../../display/sprite_data.h"

#include "ItemEntry.h"
#include "EquipmentEntry.h"
#include "WeaponEntry.h"
#include "ProjectileEntry.h"

SpriteEntry& ItemEntry::item_sprite_entry() {
	return game_sprite_data.at(item_sprite);
}
GLimage& ItemEntry::item_image() {
	return item_sprite_entry().img();
}

std::vector<ItemEntry*> game_item_data;

ItemEntry& get_item_entry(item_id id) {
	return *game_item_data.at(id);
}

void clear_item_data(std::vector<ItemEntry*>& items) {
	for (int i = 0; i < items.size(); i++) {
		delete items[i];
	}
	items.clear();
}

item_id get_item_by_name(const char* name, bool error_if_not_found) {
	for (int i = 0; i < game_item_data.size(); i++) {
		if (name == game_item_data.at(i)->name) {
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

const char* equip_type_description(ItemEntry& ientry) {
	EquipmentEntry* eentry = dynamic_cast<EquipmentEntry*>(&ientry);
	if (!eentry) {
		return "One-time Use";
	}

	switch (eentry->type) {
	case EquipmentEntry::ARMOUR:
		return "Armour";
	case EquipmentEntry::WEAPON:
		return "Weapon";
	case EquipmentEntry::PROJECTILE: {
		ProjectileEntry* pentry = dynamic_cast<ProjectileEntry*>(eentry);
		if (pentry->is_unarmed()) {
			return "Unarmed Projectile";
		} else {
			return "Projectile";
		}
	}
	case EquipmentEntry::NONE:
		return "One-time Use";
	}
	return "";
}

