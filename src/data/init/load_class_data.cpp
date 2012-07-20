/*
 * load_class_data.cpp:
 *  Load class stats and experience data
 */

#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"
#include "../../stats/stats.h"
#include "../../objects/enemy/enemy_data.h"
#include "../class_data.h"

using namespace std;

ClassSpell parse_class_spell(const YAML::Node& n) {
	ClassSpell spell;

	std::string spellname = parse_str(n["spell"]);
	spell.spell = get_spell_by_name(spellname.c_str());
	spell.xplevel_required = parse_int(n["level_needed"]);

	return spell;
}

ClassSpellProgression parse_class_spell_progression(const YAML::Node& n) {
	ClassSpellProgression progression;

	for (int i = 0; i < n.size(); i++) {
		progression.available_spells.push_back(parse_class_spell(n[i]));
	}

	return progression;
}

ClassType parse_class(const YAML::Node& n) {

//	std::string name;
//	Stats starting_stats;
//	int hp_perlevel, mp_perlevel;
//	int str_perlevel, def_perlevel, mag_perlevel;
//	float mpregen_perlevel, hpregen_perlevel;
	ClassType classtype;

	const YAML::Node& level = n["gain_per_level"];

	n["name"] >> classtype.name;
	classtype.spell_progression = parse_class_spell_progression(n["spells"]);
	classtype.starting_stats = parse_combat_stats(n["start_stats"]);

	level["mp"] >> classtype.mp_perlevel;
	level["hp"] >> classtype.hp_perlevel;

	level["strength"] >> classtype.str_perlevel;
	level["defence"] >> classtype.def_perlevel;
	level["magic"] >> classtype.mag_perlevel;
	level["willpower"] >> classtype.will_perlevel;

	level["mpregen"] >> classtype.mpregen_perlevel;
	level["hpregen"] >> classtype.hpregen_perlevel;

	return classtype;
}

void load_class_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_class_data.push_back(parse_class(node));
	value->table_set_yaml(L, game_class_data.back().name, node);
}

LuaValue load_class_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	game_class_data.clear();
	load_data_impl_template(filenames, "classes", load_class_callbackf, L,
			&ret);

	for (int i = 0; i < game_class_data.size(); i++) {
		CombatStats& cstats = game_class_data[i].starting_stats;
		cstats.class_stats.classid = i;
		cstats.init();
	}

	return ret;
}
