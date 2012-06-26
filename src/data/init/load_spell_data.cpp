/*
 * load_spell_data.cpp:
 *  Implements spell loading routines from YAML datafiles
 */
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "../game_data.h"

#include "yaml_util.h"

using namespace std;

Projectile parse_projectile_name(const YAML::Node& n) {
	std::string projectile_name = parse_str(n);
	int projectile_id = get_projectile_by_name(projectile_name.c_str());
	return Projectile(projectile_id);
}

SpellEntry parse_spell_type(const YAML::Node& n) {
	SpellEntry entry;
	entry.name = parse_str(n["name"]);
	if (yaml_has_node(n, "projectile")) {
		entry.projectile = parse_projectile_name(n["projectile"]);
	}
	entry.sprite = parse_sprite_number(n, "spr_spell");
	entry.action = LuaValue(parse_defaulted(n, "action_func", std::string()));
	return entry;
}

void load_spell_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_spell_data.push_back(parse_spell_type(node));
	value->table_set_yaml(L, game_spell_data.back().name, node);
}
LuaValue load_spell_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	game_spell_data.clear();

	load_data_impl_template(filenames, "spells", load_spell_callbackf, L, &ret);

	return ret;
}
