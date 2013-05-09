/*
 * yaml_util.cpp
 *  Utilities for loading and parsing yaml data
 */

#include <fstream>

#include <lcommon/perf_timer.h>
#include <lcommon/fatal_error.h>

#include "stats/stat_formulas.h"

#include "parse.h"
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

int parse_sprite_number(const YAML::Node & n) {
	std::string s;
	n >> s;
	return res::sprite_id(s.c_str());
}

int parse_sprite_number(const YAML::Node & n, const char *key) {
	if (!yaml_has_node(n, key))
		return -1;

	std::string s;
	n[key] >> s;
	return res::sprite_id(s.c_str());
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

const YAML::Node& operator >>(const YAML::Node& n, Colour& colour) {
	colour.r = parse_int(n[0]);
	colour.g = parse_int(n[1]);
	colour.b = parse_int(n[2]);
	if (n.size() > 3) {
		colour.a = parse_int(n[3]);
	} else {
		colour.a = 255;
	}
	return n;
}

void load_data_impl_template(const FilenameList& filenames,
		const char* resource, load_data_impl_callbackf node_callback,
		lua_State* L, LuaValue* value) {
	using namespace std;

//	std::string perfname = std::string("LOADING ") + resource;

	perf_timer_begin(FUNCNAME);

	FilenameList::const_iterator it = filenames.begin();
	for (; it != filenames.end(); ++it) {
		std::string fname = "res/" + *it;
		fstream file(fname.c_str(), fstream::in | fstream::binary);

		if (!file) {
			fprintf(stderr,
					"Fatal error: File %s not found, ensure you are running from the directory with res/ folder.\n",
					fname.c_str());
			fatal_error();
		}

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
			fprintf(stderr, "Resource file %s does not exist!\n", it->c_str());
			fflush(stderr);
		}
	}

	perf_timer_end(FUNCNAME);
}

