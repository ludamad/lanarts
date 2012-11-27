extern "C" {
#include <lua/lua.h>
}
#include <fstream>

#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../../lua/lua_yaml.h"

#include "../items/WeaponEntry.h"

#include "../items/ItemEntry.h"

#include "load_stats.h"

void parse_weapon_entry(lua_State* L, const YAML::Node& n, WeaponEntry& entry) {
	parse_equipment_entry(L, n, entry);
	entry.stackable = false;
	entry.weapon_class = parse_str(n["weapon_class"]);
	entry.attack = parse_attack(L, n);
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
	parse_weapon_entry(L, node, *entry);

	game_item_data.push_back(entry);
	if (value) {
		/* Lua loading code */
		LuaValue nodetable = lua_yaml(L, node);
		nodetable.get(L, "type") = "weapon";
		value->get(L, entry->name) = nodetable;
	}
}

void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	load_data_impl_template(filenames, "weapons", load_weapon_callbackf, L,
			itemtable);
}
