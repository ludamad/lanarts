/*
 * yaml_util.cpp
 *  Utilities for loading and parsing yaml data
 */

#include <fstream>

#include "yaml_util.h"

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
	if (!hasnode(n, key))
		return -1;

	std::string s;
	n[key] >> s;
	return get_sprite_by_name(s.c_str());
}

int parse_enemy_number(const YAML::Node& n, const char *key) {
	if (!hasnode(n, key))
		return -1;

	std::string s;
	n[key] >> s;
	for (int i = 0; i < game_enemy_data.size(); i++) {
		if (s == game_enemy_data[i].name) {
			return i;
		}
	}

	return -1;
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

StatModifier parse_modifiers(const YAML::Node& n) {
	StatModifier stat;
	optional_set(n, "strength", stat.strength_mult);
	optional_set(n, "defence", stat.defence_mult);
	optional_set(n, "magic", stat.magic_mult);
	return stat;
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

void optional_set(const YAML::Node& node, const char *key, bool & value) {
	if (hasnode(node, key)) {
		int val;
		node[key] >> val;
		value = (val != 0);
	}
}

char* tocstring(const std::string& s) {
	char *ret = new char[s.size() + 1];
	memcpy(ret, s.c_str(), s.size() + 1);
	return ret;
}

bool hasnode(const YAML::Node & n, const char *key) {
	return n.FindValue(key);
}

Stats parse_stats(const YAML::Node & n, const std::vector<Attack> & attacks) {
	Stats ret_stats;
	n["movespeed"] >> ret_stats.movespeed;
	n["hp"] >> ret_stats.max_hp;
	ret_stats.max_mp = parse_defaulted(n, "mp", 0);
	ret_stats.hpregen = parse_defaulted(n, "hpregen", 0.0);
	ret_stats.mpregen = parse_defaulted(n, "mpregen", 0.0);
	ret_stats.hp = ret_stats.max_hp;
	ret_stats.mp = ret_stats.max_hp;
	ret_stats.strength = parse_defaulted(n, "strength", 0);
	ret_stats.defence = parse_defaulted(n, "defence", 0);
	ret_stats.magic = parse_defaulted(n, "magic", 0);
	ret_stats.xpneeded = parse_defaulted(n, "xpneeded", 150);
	ret_stats.xplevel = parse_defaulted(n, "xplevel", 1);
	for (int i = 0; i < attacks.size(); i++) {
		if (!attacks[i].isprojectile)
			ret_stats.meleeatk = attacks[i];

		if (attacks[i].isprojectile)
			ret_stats.magicatk = attacks[i];

	}
	return ret_stats;
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

void load_data_impl_template(const FilenameList& filenames,
		const char* resource, load_data_impl_callbackf node_callback,
		lua_State* L, LuaValue* value) {
	using namespace std;

	FilenameList::const_iterator it = filenames.begin();
	for (; it != filenames.end(); ++it) {
		std::string fname = "res/data/" + *it;
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

