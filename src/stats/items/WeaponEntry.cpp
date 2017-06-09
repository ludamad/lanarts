/*
 * WeaponEntry.cpp:
 *  Represents enemy & player weapon (ie, for enemies, natural attacks) data
 *  loaded from the yaml
 */

#include <cstring>

#include <luawrap/luawrap.h>

#include "WeaponEntry.h"

#include "draw/SpriteEntry.h"

weapon_id get_weapon_by_name(const char* name) {
	if (strcmp(name, "Unarmed") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<WeaponEntry*>(game_item_data.get(id)));
	return (weapon_id)id;
}

WeaponEntry& get_weapon_entry(weapon_id id) {
	if (id == NO_ITEM) {
		return get_weapon_entry(get_item_by_name("Unarmed"));
	}
	ItemEntry* item = game_item_data.get(id);

	return dynamic_cast<WeaponEntry&>(*item);
}

void WeaponEntry::parse_lua_table(const LuaValue& table) {
	EquipmentEntry::parse_lua_table(table);
	weapon_class = table["type"].to_str();

	attack = parse_attack(table);
	attack.attack_sprite = item_sprite;
	if (!table["spr_attack"].isnil()) {
		attack.attack_sprite = res::sprite_id(table["spr_attack"].to_str());
	}

	uses_projectile = luawrap::defaulted(table, "uses_projectile", false);
}
