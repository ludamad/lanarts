extern "C" {
#include <lua/lua.h>
}
#include <fstream>

#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../items/WeaponEntry.h"

#include "../item_data.h"

#include "load_stats.h"

void parse_weapon_entry(const YAML::Node& n, WeaponEntry& entry) {
	parse_equipment_entry(n, entry);
	entry.stackable = false;
	entry.weapon_class = parse_str(n["weapon_class"]);
	entry.attack = parse_attack(n);
	entry.attack.attack_sprite = entry.item_sprite;
	if (yaml_has_node(n, "spr_attack")) {
		entry.attack.attack_sprite = parse_sprite_number(n, "spr_attack");
	}
	entry.uses_projectile = parse_defaulted(n, "uses_projectile", false);
	entry.type = EquipmentEntry::WEAPON;

//		entry.created_projectile = -1; TODO
//		if (yaml_has_node(n, "created_projectile")) {
//			std::string name = parse_str(n["created_projectile"]);
//			entry.created_projectile = get_projectile_by_name(name.c_str());
//		}
}
void load_weapon_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {

	const int default_cooldown = 30;
	const int default_range = 18;
	const int default_damage_radius = 3;
	WeaponEntry* entry = new WeaponEntry;
	parse_weapon_entry(node, *entry);

	game_item_data.push_back(entry);
	if (value) {
		/* Lua loading code */
		value->table_set_yaml(L, entry->name, node);
		value->table_push_value(L, entry->name);
		lua_pushstring(L, "weapon");
		lua_setfield(L, -2, "type");
		lua_pop(L, 1);
	}
}

void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	load_data_impl_template(filenames, "weapons", load_weapon_callbackf, L,
			itemtable);
}

void _load_weapon_item_entries() {
	const int default_radius = 11;

	//Create items from weapons
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<WeaponEntry*>(&ientry)) {
			WeaponEntry& entry = get_weapon_entry(i);
			//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
			_game_item_data.push_back(
					_ItemEntry(entry.name, entry.description, "", default_radius,
							entry.item_sprite, "equip", "", false,
							entry.shop_cost, _ItemEntry::ALWAYS_KNOWN,
							_ItemEntry::WEAPON, i));
		}
	}
}
