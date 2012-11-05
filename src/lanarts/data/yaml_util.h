/*
 * yaml_util.h
 *  Utilities for loading and parsing yaml data
 */

#ifndef YAML_UTIL_H_
#define YAML_UTIL_H_

#include <string>
#include <cstring>
#include <yaml-cpp/yaml.h>

#include "../objects/GameInst.h"


#include "../stats/items/WeaponEntry.h"

#include "../stats/combat_stats.h"

#include "../lanarts_defines.h"

#include "FilenameList.h"
#include "game_data.h"

//Allows for sequence merges to be defined using hash merge syntax in yaml, eg - <<: *handle, in a sequence
std::vector<const YAML::Node*> flatten_seq_mappings(const YAML::Node & n);

int parse_sprite_number(const YAML::Node & n);
int parse_sprite_number(const YAML::Node& n, const char* key);
int parse_enemy_number(const YAML::Node& n, const char* key);

char* tocstring(const std::string & s);
bool yaml_has_node(const YAML::Node& n, const char* key);
//
//void optional_set(const YAML::Node& node, const char* key, bool& value);

Range parse_range(const YAML::Node& n);

//const char* parse_cstr(const YAML::Node & n);
std::string parse_str(const YAML::Node & n);
int parse_int(const YAML::Node & n);

const YAML::Node& operator >>(const YAML::Node& n, bool& r);
const YAML::Node& operator >>(const YAML::Node& n, Range& r);
const YAML::Node& operator >>(const YAML::Node& n, FilenameList& filenames);
const YAML::Node& operator >>(const YAML::Node& n,
		Colour& colour);

template<class T>
inline T parse_defaulted(const YAML::Node& n, const char* key, const T& dflt) {
	T ret;
	if (const YAML::Node* sptr = n.FindValue(key)) {
		*sptr >> ret;
	} else
		return dflt;
	return ret;
}

template<class T>
inline void optional_set(const YAML::Node& node, const char* key, T& value) {
	if (yaml_has_node(node, key)) {
		node[key] >> value;
	}
}

typedef void (*load_data_impl_callbackf)(const YAML::Node& node, lua_State* L,
		LuaValue* value);

void load_data_impl_template(const FilenameList& filenames,
		const char* resource, load_data_impl_callbackf node_callback,
		lua_State* L = NULL, LuaValue* value = NULL);

template<class T>
void parse_map_with_defaults(const YAML::Node& node,
		std::map<std::string, T>& value_map, const char* name_key
		, T(*func)(const YAML::Node&)) {
	for (int i = 0; i < node.size(); i++) {
		const YAML::Node& n = node[i];
		if (yaml_has_node(n, "default")) {
			parse_map_with_defaults(n["default"], value_map, name_key, func);
		} else if (yaml_has_node(n, "defaults")) {
			const YAML::Node& defnode = n["defaults"];
			for (int i = 0; i < defnode.size(); i++) {
				parse_map_with_defaults(defnode[i], value_map, name_key,
						func);
			}
		} else {
			std::string name = parse_str(n[name_key]);
			value_map[name] = func(n);
		}
	}
}
template<class T>
std::vector<T> parse_named_with_defaults(const YAML::Node& node,
		const char* name_key, T(*func)(const YAML::Node&)) {
	std::map<std::string, T> value_map;
	typename std::map<std::string, T>::iterator it;

	parse_map_with_defaults(node, value_map, name_key, func);
	std::vector<T> value_list;
	for (it = value_map.begin(); it != value_map.end(); it++) {
		value_list.push_back(it->second);
	}
	return value_list;
}

#endif /* YAML_UTIL_H_ */
