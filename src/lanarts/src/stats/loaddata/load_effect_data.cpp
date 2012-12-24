#include <fstream>
#include <string>

#include <yaml-cpp/yaml.h>

#include "data/game_data.h"
#include "data/parse.h"

#include "lua/lua_yaml.h"

#include "../effect_data.h"

#include "../stats.h"

#include "load_stats.h"

using namespace std;

EffectEntry parse_effect(lua_State* L, const YAML::Node& n) {
	EffectEntry entry;
	entry.name = parse_str(n["name"]);
	entry.init_func = parse_luaexpr(L, n, "init_func");
	entry.finish_func = parse_luaexpr(L, n, "finish_func");
	entry.stat_func = parse_luaexpr(L, n, "stat_func");
	entry.attack_stat_func = parse_luaexpr(L, n, "attack_stat_func");
	entry.step_func = parse_luaexpr(L, n, "step_func");

	entry.allowed_actions.can_use_stairs = parse_defaulted(n, "can_use_stairs",
			true);
	entry.allowed_actions.can_use_rest = parse_defaulted(n, "can_use_rest",
			true);
	entry.allowed_actions.can_use_spells = parse_defaulted(n, "can_use_spells",
			true);
	entry.allowed_actions.can_use_items = parse_defaulted(n, "can_use_items",
			true);

	entry.effected_colour = parse_defaulted(n, "effected_colour", Colour());
	entry.effected_sprite = parse_sprite_number(n, "effected_sprite");
	entry.additive_duration = parse_defaulted(n, "additive_duration", false);
	entry.fade_out = parse_defaulted(n, "fade_out", 100);
	return entry;
}

void load_effect_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	EffectEntry entry = parse_effect(L, node);
	game_effect_data.push_back(entry);
	(*value)[entry.name] = node;
}

LuaValue load_effect_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret(L);
	ret.newtable();

	game_effect_data.clear();

	load_data_impl_template(filenames, "effects", load_effect_callbackf, L,
			&ret);

	return ret;
}
