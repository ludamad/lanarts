#include <fstream>
#include "yaml_util.h"
#include "../game_data.h"
#include "../item_data.h"

using namespace std;

void load_weapon_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	const int default_cooldown = 30;
	const int default_range = 18;
	const int default_damage_radius = 3;

	int issolid = 0;

	sprite_id item_spr = parse_sprite_number(node, "spr_item");
	sprite_id atk_spr = item_spr;
	if (hasnode(node, "spr_attack")) {
		atk_spr = parse_sprite_number(node, "spr_attack");
	}

	WeaponEntry entry(parse_str(node["name"]), parse_str(node["weapon_class"]),
			parse_defaulted(node, "projectile", 0),
			parse_defaulted(node, "max_targets", 1),
			parse_range(node["damage"]), parse_modifiers(node["modifiers"]),
			parse_defaulted(node, "range", default_range),
			parse_defaulted(node, "cooldown", default_cooldown),
			parse_defaulted(node, "damage_area_radius", default_damage_radius),
			item_spr, atk_spr, parse_defaulted(node, "proj_speed", 0));

	game_weapon_data.push_back(entry);
	if (value)
		value->table_set_yaml(L, game_weapon_data.back().name, &node);
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
				ItemEntry(wtype.name, default_radius, wtype.item_sprite,
						"equip", "", false, ItemEntry::WEAPON, i));
	}
}

void load_projectile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	const int default_cooldown = 30;
	const int default_damage_radius = 3;

	ProjectileEntry entry;
	entry.name = parse_str(node["name"]);
	entry.damage = parse_defaulted(node, "damage", Range(0,0));
	entry.damage_multiplier = parse_defaulted(node, "modifiers", StatModifier());
	entry.damage_added = parse_defaulted(node, "damage_added", Range(0,0));

	entry.item_sprite = parse_sprite_number(node, "spr_item");
	entry.attack_sprite = entry.item_sprite;
	if (hasnode(node, "spr_attack")) {
		entry.attack_sprite = parse_sprite_number(node, "spr_attack");
	}
	entry.break_chance = parse_defaulted(node, "break_chance", 0);
	entry.weapon_class = parse_str(node["weapon_class"]);

	entry.speed = parse_defaulted(node, "speed", 4);
	entry.cooldown = parse_defaulted(node, "cooldown", 0);

	game_projectile_data.push_back(entry);
	if (value)
		value->table_set_yaml(L, game_projectile_data.back().name, &node);
}
void load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	game_projectile_data.clear();
	load_data_impl_template(filenames, "projectiles", load_projectile_callbackf,
			L, itemtable);
}

void load_projectile_item_entries() {
	const int default_radius = 15;

	//Create items from projectiles
	for (int i = 0; i < game_projectile_data.size(); i++) {
		ProjectileEntry& ptype = game_projectile_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(ptype.name, default_radius, ptype.item_sprite,
						"equip", "", true, ItemEntry::PROJECTILE, i));
	}
}
