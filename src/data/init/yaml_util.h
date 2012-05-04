/*
 * yaml_util.h
 *
 *  Created on: Mar 26, 2012
 *      Author: 100397561
 */

#ifndef YAML_UTIL_H_
#define YAML_UTIL_H_
#include <string>
#include <cstring>
#include <yaml-cpp/yaml.h>
#include "../../world/objects/GameInst.h"
#include "../game_data.h"
#include "../../gamestats/Stats.h"
#include "../weapon_data.h"

struct GenRange {
	int min, max;
};

//Allows for sequence merges to be defined using hash merge syntax in yaml, eg - <<: *handle, in a sequence
std::vector<const YAML::Node* > flatten_seq_mappings(const YAML::Node & n);

int parse_sprite_number(const YAML::Node & n, const char *key);
int parse_enemy_number(const YAML::Node & n, const char *key);

char* tocstring(const std::string & s);
bool hasnode(const YAML::Node & n, const char *key);

void optional_set(const YAML::Node & node, const char *key, bool & value);

Stats parse_stats(const YAML::Node & n, const std::vector<Attack> & attacks);
GenRange parse_range(const YAML::Node & n);
StatModifier parse_modifiers(const YAML::Node & n);
const char *parse_cstr(const YAML::Node & n);

template <class T>
inline T parse_defaulted(const YAML::Node& n, const char* key, const T& dflt){
	T ret;
	if(const YAML::Node* sptr = n.FindValue(key)){
		*sptr >> ret;
	} else return dflt;
	return ret;
}

template <class T>
inline void optional_set(const YAML::Node& node, const char* key, T& value){
	if (hasnode(node, key)){
		node[key] >> value;
	}
}

#endif /* YAML_UTIL_H_ */
