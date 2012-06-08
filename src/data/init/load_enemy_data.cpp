#include <fstream>
#include <yaml-cpp/yaml.h>

#include "../../gamestats/stats.h"

#include "../enemy_data.h"
#include "../game_data.h"

#include "yaml_util.h"

using namespace std;

Attack parse_attack(const YAML::Node& n) {
	Attack ret;
	ret.canuse = true;
	ret.cooldown = parse_defaulted(n, "cooldown", 70);
	ret.range = parse_defaulted(n, "range", 20);
	ret.projectile_speed = parse_defaulted(n, "projectile_speed", 0);
	n["damage"] >> ret.damage;
	ret.isprojectile = parse_defaulted(n, "uses_projectile", false);
	ret.attack_sprite = parse_sprite_number(n, "sprite");
	return ret;
}
EnemyEntry parse_enemy_type(const YAML::Node& n) {
	EnemyEntry entry;

	const YAML::Node& anodes = n["attacks"];
	vector<Attack> attacks;
	for (int i = 0; i < anodes.size(); i++) {
		attacks.push_back(parse_attack(anodes[i]));
	}
//	std::string name, appear_msg, defeat_msg;
//	int radius;
//	int xpaward;
//	int sprite_number;
//	Stats basestats;
//	bool unique;
//
//	LuaValue init_event, step_event;

	entry.name = parse_str(n["name"]);

	entry.appear_msg = parse_defaulted(n, "appear_message", std::string());
	entry.defeat_msg = parse_defaulted(n, "defeat_message", std::string());

	entry.radius = parse_int(n["radius"]);
	entry.xpaward = parse_int(n["xpaward"]);

	entry.sprite_number = parse_sprite_number(n, "sprite");
	entry.basestats = parse_stats(n["stats"], attacks);
	entry.unique = parse_defaulted(n, "unique", false);

	entry.init_event = LuaValue(parse_defaulted(n, "init_func", std::string()));
	entry.step_event = LuaValue(parse_defaulted(n, "step_func", std::string()));

	return entry;
}

void load_enemy_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_enemy_data.push_back(parse_enemy_type(node));
	value->table_set_yaml(L, game_enemy_data.back().name, &node);
}

LuaValue load_enemy_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	game_enemy_data.clear();
	load_data_impl_template(filenames, "enemies", load_enemy_callbackf, L,
			&ret);

	return ret;
}
