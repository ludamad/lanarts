#include <fstream>
#include <yaml-cpp/yaml.h>

#include "../../data/game_data.h"

#include "../../data/parse.h"

#include "../../lua/lua_yaml.h"
#include "stats/stats.h"
#include "stats/loaddata/load_stats.h"

#include "../enemy/EnemyEntry.h"

using namespace std;

EnemyEntry parse_enemy_type(lua_State* L, const YAML::Node& n) {
	EnemyEntry entry;

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

	entry.init_event = parse_luacode(L, n, "init_func");
	entry.step_event = parse_luacode(L, n, "step_func");

	return entry;
}

void load_enemy_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	EnemyEntry entry = parse_enemy_type(L, node);
	game_enemy_data.push_back(entry);

	(*value)[entry.name] = node;
}

LuaValue load_enemy_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret(L);
	ret.newtable();

	game_enemy_data.clear();
	load_data_impl_template(filenames, "enemies", load_enemy_callbackf, L,
			&ret);

	return ret;
}
