#include <fstream>
#include "yaml_util.h"
#include "../game_data.h"
#include "../item_data.h"

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
	entry.weapon_class = parse_str(node["weapon_class"]);
	entry.uses_projectile = parse_defaulted(node, "uses_projectile", false);
	entry.max_targets = parse_defaulted(node, "max_targets", 1);

	entry.damage = parse_defaulted(node, "damage", CoreStatMultiplier());
	entry.power = parse_defaulted(node, "power", CoreStatMultiplier());

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

	game_weapon_data.push_back(entry);
	if (value)
		value->table_set_yaml(L, game_weapon_data.back().name, node);
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
		WeaponEntry& wtype = game_weapon_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(wtype.name, wtype.description, "", default_radius,
						wtype.item_sprite, "equip", "", false,
						ItemEntry::WEAPON, i));
	}
}

void load_projectile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ProjectileEntry entry;
	entry.name = parse_str(node["name"]);
	entry.description = parse_defaulted(node, "description", std::string());

	entry.damage = parse_defaulted(node, "damage", CoreStatMultiplier());
	entry.power = parse_defaulted(node, "power", CoreStatMultiplier());

	entry.percentage_magic = parse_magic_percentage(node, "damage_type");

	entry.item_sprite = parse_sprite_number(node, "spr_item");
	entry.attack_sprite = entry.item_sprite;
	if (yaml_has_node(node, "spr_attack")) {
		entry.attack_sprite = parse_sprite_number(node, "spr_attack");
	}
	entry.drop_chance = parse_defaulted(node, "drop_chance", 0);
	entry.weapon_class = parse_str(node["weapon_class"]);

	entry.speed = parse_defaulted(node, "speed", 4.0f);
	entry.number_of_target_bounces = parse_defaulted(node,
			"number_of_target_bounces", 0);
	entry.cooldown = parse_defaulted(node, "cooldown", 0);
	entry.radius = parse_defaulted(node, "radius", 5);
	entry.range = parse_defaulted(node, "range", 0);

	game_projectile_data.push_back(entry);
	if (value)
		value->table_set_yaml(L, game_projectile_data.back().name, node);
}
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemtable) {
	LuaValue ret;
	ret.table_initialize(L);

	game_projectile_data.clear();
	load_data_impl_template(filenames, "projectiles", load_projectile_callbackf,
			L, &itemtable);

	for (int i = 0; i < game_projectile_data.size(); i++) {
		itemtable.table_push_value(L, game_projectile_data[i].name.c_str());
		ret.table_pop_value(L, game_projectile_data[i].name.c_str());
	}
	return ret;
}

void load_projectile_item_entries() {
	const int default_radius = 15;

	//Create items from projectiles
	for (int i = 0; i < game_projectile_data.size(); i++) {
		ProjectileEntry& ptype = game_projectile_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(ptype.name, ptype.description, "", default_radius,
						ptype.item_sprite, "equip", "", true,
						ItemEntry::PROJECTILE, i));
	}
}
