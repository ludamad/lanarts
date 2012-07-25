/*
 * yaml_util.cpp
 *  Utilities for loading and parsing yaml data
 */

#include <fstream>

#include "yaml_util.h"

#include "../stats/stat_formulas.h"

std::vector<const YAML::Node*> flatten_seq_mappings(const YAML::Node & n) {
	std::vector<const YAML::Node*> ret;
	for (int i = 0; i < n.size(); i++) {
		if (n[i].Type() == YAML::NodeType::Map) {
			const YAML::Node & map = n[i];
			YAML::Iterator it = map.begin();

			for (; it != map.end(); ++it) {
				ret.push_back(&*it);
			}
		} else
			ret.push_back(&n[i]);

	}
	return ret;
}

int parse_sprite_number(const YAML::Node & n, const char *key) {
	if (!yaml_has_node(n, key))
		return -1;

	std::string s;
	n[key] >> s;
	return get_sprite_by_name(s.c_str());
}

int parse_enemy_number(const YAML::Node& n, const char *key) {
	if (!yaml_has_node(n, key))
		return -1;

	std::string s;
	n[key] >> s;

	return get_enemy_by_name(s.c_str());
}

const char* parse_cstr(const YAML::Node& n) {
	std::string s;
	n >> s;
	return tocstring(s);
}
int parse_int(const YAML::Node& n) {
	int val;
	n >> val;
	return val;
}
std::string parse_str(const YAML::Node& n) {
	std::string s;
	n >> s;
	return s;
}

Range parse_range(const YAML::Node& n) {
	Range gr;
	if (n.Type() == YAML::NodeType::Sequence) {
		std::vector<int> components;
		n >> components;
		gr.min = components[0];
		gr.max = components[1];
	} else {
		n >> gr.min;
		gr.max = gr.min;
	}
	return gr;
}

char* tocstring(const std::string& s) {
	char *ret = new char[s.size() + 1];
	memcpy(ret, s.c_str(), s.size() + 1);
	return ret;
}

bool yaml_has_node(const YAML::Node & n, const char *key) {
	return n.FindValue(key);
}

const YAML::Node & operator >>(const YAML::Node& n, Range& r) {
	r = parse_range(n);
	return n;
}

const YAML::Node & operator >>(const YAML::Node& n, FilenameList & filenames) {
	if (n.Type() == YAML::NodeType::Scalar) {
		filenames.resize(1);
		n >> filenames[0];
	} else {
		filenames.resize(n.size());
		for (int i = 0; i < n.size(); i++) {
			n[i] >> filenames[i];
		}
	}
	return n;
}

const YAML::Node & operator >>(const YAML::Node& n, bool& r) {
	std::string val = parse_str(n);
	if (val == "yes")
		r = true;
	else if (val == "no")
		r = false;
	else
		throw YAML::InvalidScalar(n.GetMark());
	return n;
}

static Item parse_as_item(const YAML::Node& n) {
	std::string s = parse_str(n);
	return Item(get_item_by_name(s.c_str()));
}

Inventory parse_inventory(const YAML::Node& n) {
	Inventory ret;
	for (int i = 0; i < n.size(); i++) {
		const YAML::Node& slot = n[i];
		ret.add(parse_as_item(slot["item"]),
				parse_defaulted(slot, "amount", 1));
	}
	return ret;
}
Equipment parse_equipment(const YAML::Node& n) {
	Equipment ret;
	ret.inventory = parse_inventory(n["inventory"]);
	return ret;
}

CombatStats parse_combat_stats(const YAML::Node& n) {
	CombatStats ret;
	ClassStats& class_stats = ret.class_stats;
	CoreStats& core = ret.core;

	n["movespeed"] >> ret.movespeed;

	if (yaml_has_node(n, "equipment")) {
		ret.equipment = parse_equipment(n["equipment"]);
	}
	core.max_mp = parse_defaulted(n, "mp", 0);
	core.max_hp = parse_int(n["hp"]);
	core.hpregen = parse_defaulted(n, "hpregen", 0.0);
	core.mpregen = parse_defaulted(n, "mpregen", 0.0);
	core.hp = core.max_hp;
	core.mp = core.max_hp;

	core.strength = parse_defaulted(n, "strength", 0);
	core.defence = parse_defaulted(n, "defence", 0);

	core.magic = parse_defaulted(n, "magic", 0);
	core.willpower = parse_defaulted(n, "willpower", 0);

	core.physical_reduction = parse_defaulted(n, "reduction", 0);
	core.magic_reduction = parse_defaulted(n, "magic_reduction", 0);

	class_stats.xpneeded = parse_defaulted(n, "xpneeded",
			experience_needed_formula(1));
	class_stats.xplevel = parse_defaulted(n, "xplevel", 1);
	ret.attacks = parse_defaulted(n, "attacks", std::vector<AttackStats>());

	return ret;
}

const YAML::Node& operator >>(const YAML::Node& n, CoreStatMultiplier& sm) {
	sm.base = parse_defaulted(n, "base", Range(0, 0));
	sm.strength = parse_defaulted(n, "strength", 0.0f);
	sm.magic = parse_defaulted(n, "magic", 0.0f);
	sm.defence = parse_defaulted(n, "defence", 0.0f);
	sm.willpower = parse_defaulted(n, "willpower", 0.0f);
	return n;
}

const YAML::Node& operator >>(const YAML::Node& n,
		std::vector<AttackStats>& attacks) {
	for (int i = 0; i < n.size(); i++) {
		attacks.push_back(parse_attack(n[i]));
	}
	return n;
}

AttackStats parse_attack(const YAML::Node & n) {
	std::string name;
	AttackStats ret;

	if (yaml_has_node(n, "weapon")) {
		name = parse_str(n["weapon"]);
		ret.weapon = Weapon(get_weapon_by_name(name.c_str()));
	}
	if (yaml_has_node(n, "projectile")) {
		name = parse_str(n["projectile"]);
		ret.projectile = Projectile(get_projectile_by_name(name.c_str()));
	}
	return ret;
}

void load_data_impl_template(const FilenameList& filenames,
		const char* resource, load_data_impl_callbackf node_callback,
		lua_State* L, LuaValue* value) {
	using namespace std;

	FilenameList::const_iterator it = filenames.begin();
	for (; it != filenames.end(); ++it) {
		std::string fname = "res/" + *it;
		fstream file(fname.c_str(), fstream::in | fstream::binary);

		if (file) {
			try {
				YAML::Parser parser(file);
				YAML::Node root;

				parser.GetNextDocument(root);
				const YAML::Node& cnode = root[resource];
				for (int i = 0; i < cnode.size(); i++) {
					node_callback(cnode[i], L, value);
				}
				file.close();
			} catch (const YAML::Exception& parse) {
				printf("%s parsed incorrectly! \n", resource);
				printf("Error while loading from file %s:\n%s\n", fname.c_str(),
						parse.what());
				fflush(stdout);
			}
		} else {
			printf("file '%s' could not be loaded!\n", it->c_str());
			fflush(stdout);
		}
	}
}

