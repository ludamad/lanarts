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

void load_projectile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ProjectileEntry entry;
	entry.name = parse_str(node["name"]);
	entry.description = parse_defaulted(node, "description", std::string());
	entry.shop_cost = parse_defaulted(node, "shop_cost", Range());

	entry.damage = parse_defaulted(node, "damage", CoreStatMultiplier());
	entry.power = parse_defaulted(node, "power", CoreStatMultiplier());

	entry.percentage_magic = parse_magic_percentage(node, "damage_type");
	entry.resist_modifier = parse_defaulted(node, "resist_modifier", 1.0f);

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
	entry.can_wall_bounce = parse_defaulted(node, "can_wall_bounce", false);
	entry.cooldown = parse_defaulted(node, "cooldown", 0);
	entry.radius = parse_defaulted(node, "radius", 5);
	entry.range = parse_defaulted(node, "range", 0);

	entry.on_hit_func = LuaValue(
			parse_defaulted(node, "on_hit_func", std::string()));

	game_projectile_data.push_back(entry);

	/* Lua loading code */
	value->table_set_yaml(L, entry.name, node);
	value->table_push_value(L, entry.name);
	lua_pushstring(L, "projectile");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);
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
		ProjectileEntry& entry = game_projectile_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(entry.name, entry.description, "", default_radius,
						entry.item_sprite, "equip", "", true, entry.shop_cost,
						ItemEntry::ALWAYS_KNOWN, ItemEntry::PROJECTILE, i));
	}
}
