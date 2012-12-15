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

#endif /* YAML_UTIL_H_ */
