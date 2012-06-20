#include <fstream>
#include <string>

#include <yaml-cpp/yaml.h>

#include "../../gamestats/stats.h"

#include "../effect_data.h"
#include "../game_data.h"

#include "yaml_util.h"

using namespace std;

EffectEntry parse_effect(const YAML::Node& n) {
	return EffectEntry(parse_str(n["name"]),
			parse_defaulted(n, "stat_func", std::string()),
			parse_defaulted(n, "additive_duration", true));
}

void load_effect_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_effect_data.push_back(parse_effect(node));
}

LuaValue load_effect_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;
	game_effect_data.clear();

	load_data_impl_template(filenames, "effects", load_effect_callbackf, L,
			&ret);

	return ret;
}
