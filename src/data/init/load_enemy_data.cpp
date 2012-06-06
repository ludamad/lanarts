#include <fstream>
#include <yaml-cpp/yaml.h>

#include "../../gamestats/Stats.h"

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
	ret.isprojectile = parse_defaulted(n, "projectile", 0);
	ret.attack_sprite = parse_sprite_number(n, "sprite");
	return ret;
}
EnemyEntry parse_enemy_type(const YAML::Node& n) {
	int sprite_number;
	int radius;
	int xpaward;
	n["xpaward"] >> xpaward;
	n["radius"] >> radius;
	const YAML::Node& anodes = n["attacks"];
	vector<Attack> attacks;
	for (int i = 0; i < anodes.size(); i++) {
		attacks.push_back(parse_attack(anodes[i]));
	}

	return EnemyEntry(parse_str(n["name"]), radius, xpaward,
			parse_sprite_number(n, "sprite"), parse_stats(n["stats"], attacks),
			parse_defaulted(n, "init_func", std::string()),
			parse_defaulted(n, "step_func", std::string()),
			parse_defaulted(n, "unique", 0));
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
