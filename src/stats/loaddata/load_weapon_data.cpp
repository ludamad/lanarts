#include <fstream>

#include "../item_data.h"
#include "../../data/game_data.h"

#include "../../data/yaml_util.h"

extern "C" {
#include <lua/lua.h>
}

using namespace std;
static float parse_magic_percentage(const YAML::Node& node, const char* key) {
	if (yaml_has_node(node, key)) {
		float magic_percentage = parse_defaulted(node[key], "magic", 0.0f);
		float physical_percentage = parse_defaulted(node[key], "physical",
				0.0f);
		float sum = magic_percentage + physical_percentage;
		LANARTS_ASSERT(sum > .99f && sum < 1.01f);
		return magic_percentage;
	} else {
		return 0.0f;
	}
}

void load_weapon_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
//	std::string name;
//	std::string weapon_class;
//	bool projectile;
//	int max_targets;
//	CoreStatMultiplier power, damage;
//	float percentage_magic;//Conversely the rest is percentage physical
//
//	int range, dmgradius, cooldown;
//	sprite_id item_sprite, attack_sprite;
//	projectile_id created_projectile; // for infinite ammo weapons

	const int default_cooldown = 30;
	const int default_range = 18;
	const int default_damage_radius = 3;

	WeaponEntry entry;
	entry.name = parse_str(node["name"]);
	entry.description = parse_defaulted(node, "description", std::string());
	entry.shop_cost = parse_defaulted(node, "shop_cost", Range());

	entry.weapon_class = parse_str(node["weapon_class"]);
	entry.uses_projectile = parse_defaulted(node, "uses_projectile", false);
	entry.max_targets = parse_defaulted(node, "max_targets", 1);

	entry.damage = parse_defaulted(node, "damage", CoreStatMultiplier());
	entry.power = parse_defaulted(node, "power", CoreStatMultiplier());
	entry.resist_modifier = parse_defaulted(node, "resist_modifier", 1.0f);

	entry.percentage_magic = parse_magic_percentage(node, "damage_type");
	entry.range = parse_defaulted(node, "range", default_range);
	entry.cooldown = parse_defaulted(node, "cooldown", default_cooldown);
	entry.dmgradius = parse_defaulted(node, "damage_area_radius",
			default_damage_radius);

	entry.item_sprite = parse_sprite_number(node, "spr_item");
	entry.attack_sprite = entry.item_sprite;
	if (yaml_has_node(node, "spr_attack")) {
		entry.attack_sprite = parse_sprite_number(node, "spr_attack");
	}

	entry.created_projectile = -1;
	if (yaml_has_node(node, "created_projectile")) {
		std::string name = parse_str(node["created_projectile"]);
		entry.created_projectile = get_projectile_by_name(name.c_str());
	}

	entry.on_hit_func = LuaValue(
			parse_defaulted(node, "on_hit_func", std::string()));

	game_weapon_data.push_back(entry);
	if (value) {
		const std::string& name = game_weapon_data.back().name;
		value->table_set_yaml(L, name, node);
		value->table_push_value(L, name);
		lua_pushstring(L, "weapon");
		lua_setfield(L, -2, "type");
		lua_pop(L, 1);
	}
}

void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	game_weapon_data.clear();
	load_data_impl_template(filenames, "weapons", load_weapon_callbackf, L,
			itemtable);
}

void load_weapon_item_entries() {
	const int default_radius = 11;

	//Create items from weapons
	for (int i = 0; i < game_weapon_data.size(); i++) {
		WeaponEntry& entry = game_weapon_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(entry.name, entry.description, "", default_radius,
						entry.item_sprite, "equip", "", false, entry.shop_cost,
						ItemEntry::ALWAYS_KNOWN, ItemEntry::WEAPON, i));
	}
}
