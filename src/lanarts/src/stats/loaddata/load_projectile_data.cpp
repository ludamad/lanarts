#include <lua.hpp>

#include <fstream>

#include <luawrap/luawrap.h>

#include "../items/ProjectileEntry.h"
#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"

#include "../items/ItemEntry.h"
#include "load_stats.h"

using namespace std;

void parse_projectile_entry(lua_State* L, const YAML::Node& n, ProjectileEntry& entry) {
	parse_equipment_entry(L, n, entry);
	entry.stackable = true;
	entry.attack = parse_attack(L, n);
	entry.attack.attack_sprite = entry.item_sprite;
	if (yaml_has_node(n, "spr_attack")) {
		entry.attack.attack_sprite = parse_sprite_number(n, "spr_attack");
	}

	entry.drop_chance = parse_optional(n, "drop_chance", 0);
	entry.weapon_class = parse_str(n["weapon_class"]);

	entry.speed = parse_optional(n, "speed", 4.0f);
	entry.number_of_target_bounces = parse_optional(n,
			"number_of_target_bounces", 0);
	entry.can_wall_bounce = parse_optional(n, "can_wall_bounce", false);
	entry.radius = parse_optional(n, "radius", 5);
	entry.type = EquipmentEntry::AMMO;
}

static void load_projectile(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ProjectileEntry* entry = new ProjectileEntry;
	parse_projectile_entry(L, node, *entry);

	game_item_data.push_back(entry);
	/* Lua loading code */

	LuaValue nodetable = lua_yaml(L, node);
	nodetable["type"] = "projectile";
	(*value)[entry->name] = nodetable;
}
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemtable) {
	LuaValue ret(L);
	ret.newtable();

	load_data_impl_template(filenames, "projectiles", load_projectile, L,
			&itemtable);
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<ProjectileEntry*>(&ientry)) {
			ProjectileEntry& entry = get_projectile_entry(i);

			ret[entry.name] = itemtable[entry.name];
		}
	}
	return ret;
}
