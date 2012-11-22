extern "C" {
#include <lua/lua.h>
}

#include <fstream>

#include "../items/ProjectileEntry.h"
#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../../lua/lua_yaml.h"

#include "../items/ItemEntry.h"
#include "load_stats.h"

using namespace std;

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

static void load_projectile(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ProjectileEntry* entry = new ProjectileEntry;
	parse_projectile_entry(node, *entry);

	game_item_data.push_back(entry);
	/* Lua loading code */

	LuaValue nodetable = lua_yaml(L, node);
	value->get(L, entry->name) = nodetable;
	nodetable.get(L, "type") = "projectile";
}
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemtable) {
	LuaValue ret;
	ret.table_initialize(L);

	load_data_impl_template(filenames, "projectiles", load_projectile, L,
			&itemtable);
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<ProjectileEntry*>(&ientry)) {
			ProjectileEntry& entry = get_projectile_entry(i);

			ret.get(L, entry.name) = itemtable.get(L, entry.name);
		}
	}
	return ret;
}