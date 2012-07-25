#include <fstream>
#include <string>

#include <yaml-cpp/yaml.h>

#include "../stats.h"

#include "../effect_data.h"
#include "../../data/game_data.h"

#include "../../data/yaml_util.h"

using namespace std;

EffectEntry parse_effect(const YAML::Node& n) {
	EffectEntry entry;
	entry.name = parse_str(n["name"]);
	entry.init_func = LuaValue(parse_defaulted(n, "init_func", std::string()));
	entry.finish_func = LuaValue(
			parse_defaulted(n, "finish_func", std::string()));
	entry.stat_func = LuaValue(parse_defaulted(n, "stat_func", std::string()));
	entry.step_func = LuaValue(parse_defaulted(n, "step_func", std::string()));
	entry.additive_duration = parse_defaulted(n, "additive_duration", false);
	return entry;
}

void load_effect_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_effect_data.push_back(parse_effect(node));
	value->table_set_yaml(L, parse_str(node["name"]), node);
}

LuaValue load_effect_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;
	game_effect_data.clear();

	load_data_impl_template(filenames, "effects", load_effect_callbackf, L,
			&ret);

	return ret;
}
