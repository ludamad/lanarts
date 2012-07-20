#include <fstream>
#include <yaml-cpp/yaml.h>

#include "../../stats/stats.h"

#include "../../objects/enemy/enemy_data.h"
#include "../game_data.h"

#include "yaml_util.h"

using namespace std;

EnemyEntry parse_enemy_type(const YAML::Node& n) {
	EnemyEntry entry;

//	std::string name, appear_msg, defeat_msg;
//	int radius;
//	int xpaward;
//	sprite_id sprite_number, death_sprite;
//	Stats basestats;
//	bool unique;
//
//	LuaValue init_event, step_event;

	entry.name = parse_str(n["name"]);
	entry.description = parse_defaulted(n, "description", std::string());

	entry.appear_msg = parse_defaulted(n, "appear_message", std::string());
	entry.defeat_msg = parse_defaulted(n, "defeat_message", std::string());

	entry.radius = parse_int(n["radius"]);
	entry.xpaward = parse_int(n["xpaward"]);

	entry.enemy_sprite = parse_sprite_number(n, "sprite");
	entry.death_sprite = parse_sprite_number(n, "death_sprite");
	entry.basestats = parse_combat_stats(n["stats"]);
	entry.unique = parse_defaulted(n, "unique", false);

	entry.init_event = LuaValue(parse_defaulted(n, "init_func", std::string()));
	entry.step_event = LuaValue(parse_defaulted(n, "step_func", std::string()));

	return entry;
}

void load_enemy_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_enemy_data.push_back(parse_enemy_type(node));
	value->table_set_yaml(L, game_enemy_data.back().name, node);
}

LuaValue load_enemy_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	game_enemy_data.clear();
	load_data_impl_template(filenames, "enemies", load_enemy_callbackf, L,
			&ret);

	return ret;
}
