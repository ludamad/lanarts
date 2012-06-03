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

	Range damage = parse_range(node["damage"]);
	WeaponEntry entry(parse_str(node["name"]),
			parse_defaulted(node, "projectile", 0),
			parse_defaulted(node, "max_targets", 1), damage.min, damage.max,
			parse_modifiers(node["modifiers"]),
			parse_defaulted(node, "range", default_range),
			parse_defaulted(node, "cooldown", default_cooldown),
			parse_defaulted(node, "damage_area_radius", default_damage_radius),
			parse_sprite_number(node, "spr_weap"),
			parse_sprite_number(node, "spr_attack"),
			parse_defaulted(node, "proj_speed", 0));

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

void load_weapon_item_entries(lua_State* L) {
	const int default_radius = 11;

	//Create items from weapons
	for (int i = 0; i < game_weapon_data.size(); i++) {
		WeaponEntry* wtype = &game_weapon_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(wtype->name, default_radius, wtype->weapon_sprite,
						"equip", "", false, i));
	}
}
