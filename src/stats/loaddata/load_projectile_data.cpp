extern "C" {
#include <lua/lua.h>
}

#include <fstream>

#include "../items/ProjectileEntry.h"
#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../item_data.h"
#include "load_stats.h"

using namespace std;

void _load_projectile_item_entries() {
	const int default_radius = 15;

	//Create items from projectiles
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<ProjectileEntry*>(&ientry)) {
			ProjectileEntry& entry = get_projectile_entry(i);
			//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
			_game_item_data.push_back(
					_ItemEntry(entry.name, entry.description, "",
							default_radius, entry.item_sprite, "equip", "",
							true, entry.shop_cost, _ItemEntry::ALWAYS_KNOWN,
							_ItemEntry::PROJECTILE, i));
		}
	}
}

void parse_projectile_entry(const YAML::Node& n, ProjectileEntry& entry) {
	parse_equipment_entry(n, entry);
	entry.stackable = true;
	entry.attack = parse_attack(n);
	entry.attack.attack_sprite = entry.item_sprite;
	if (yaml_has_node(n, "spr_attack")) {
		entry.attack.attack_sprite = parse_sprite_number(n, "spr_attack");
	}

	entry.drop_chance = parse_defaulted(n, "drop_chance", 0);
	entry.weapon_class = parse_str(n["weapon_class"]);

	entry.speed = parse_defaulted(n, "speed", 4.0f);
	entry.number_of_target_bounces = parse_defaulted(n,
			"number_of_target_bounces", 0);
	entry.can_wall_bounce = parse_defaulted(n, "can_wall_bounce", false);
	entry.radius = parse_defaulted(n, "radius", 5);
	entry.type = EquipmentEntry::PROJECTILE;
}

void load_projectile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ProjectileEntry* entry = new ProjectileEntry;
	parse_projectile_entry(node, *entry);

	game_item_data.push_back(entry);
	/* Lua loading code */
	value->table_set_yaml(L, entry->name, node);
	value->table_push_value(L, entry->name);
	lua_pushstring(L, "projectile");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);

}
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemtable) {
	LuaValue ret;
	ret.table_initialize(L);

	load_data_impl_template(filenames, "projectiles", load_projectile_callbackf,
			L, &itemtable);
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<ProjectileEntry*>(&ientry)) {
			ProjectileEntry& entry = get_projectile_entry(i);
			itemtable.table_push_value(L, entry.name.c_str());
			ret.table_pop_value(L, entry.name.c_str());
		}
	}
	return ret;
}
