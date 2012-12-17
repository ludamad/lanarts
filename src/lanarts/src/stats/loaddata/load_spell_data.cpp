/*
 * load_spell_data.cpp:
 *  Implements spell loading routines from YAML datafiles
 */
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "data/game_data.h"

#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua/lua_yaml.h"

#include "load_stats.h"

using namespace std;

Projectile parse_projectile_name(const YAML::Node& n) {
	std::string projectile_name = parse_str(n);
	projectile_id id = get_projectile_by_name(projectile_name.c_str());
	return Projectile(id);
}

const char* default_autotarget_func = "spell_choose_target";

SpellEntry parse_spell_type(lua_State* L, const YAML::Node& n) {
	SpellEntry entry;
	entry.name = parse_str(n["name"]);
	entry.description = parse_defaulted(n, "description", std::string());
	entry.sprite = parse_sprite_number(n, "spr_spell");
	entry.mp_cost = parse_int(n["mp_cost"]);
	entry.cooldown = parse_int(n["cooldown"]);
	if (yaml_has_node(n, "projectile")) {
		entry.projectile = parse_projectile_name(n["projectile"]);
	}
	entry.action_func = parse_luaexpr(L, n, "action_func");
	entry.autotarget_func = parse_luaexpr(L, n, "autotarget_func", default_autotarget_func);
	entry.prereq_func = parse_luaexpr(L, n, "prereq_func");
	entry.can_cast_with_cooldown = parse_defaulted(n, "can_cast_with_cooldown",
			false);
	entry.can_cast_with_held_key = parse_defaulted(n, "can_cast_with_held_key",
			true);
	return entry;
}

void load_spell_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	SpellEntry entry = parse_spell_type(L, node);
	game_spell_data.push_back(entry);

	(*value)[entry.name] = node;
}
LuaValue load_spell_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret(L);
	ret.newtable();

	game_spell_data.clear();

	load_data_impl_template(filenames, "spells", load_spell_callbackf, L, &ret);

	return ret;
}
