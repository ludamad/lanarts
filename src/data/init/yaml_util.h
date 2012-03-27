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

struct GenRange {
	int min, max;
};

static char* tocstring(const std::string& s){
	char* ret = new char[s.size()+1];
	memcpy(ret, s.c_str(), s.size()+1);
	return ret;
}

inline int get_sprite_number(const std::string& s){
	for (int i = 0; i < game_sprite_data.size(); i++){
		if (s == game_sprite_data[i].name){
			return i;
		}
	}
	return -1;
}

inline bool hasnode(const YAML::Node& n, const char* key){
	return n.FindValue(key);
}

inline int parse_sprite_number(const YAML::Node& n, const char* key){
	if (!hasnode(n,key)) return -1;
	std::string s;
	n[key] >> s;
	return get_sprite_number(s);
}
template <class T>
inline T parse_defaulted(const YAML::Node& n, const char* key, const T& dflt){
	T ret;
	if(const YAML::Node* sptr = n.FindValue(key)){
		*sptr >> ret;
	} else return dflt;
	return ret;
}


inline GenRange parse_range(const YAML::Node& n){
	GenRange gr;
	if (n.Type() == YAML::NodeType::Sequence){
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

inline const char* parse_cstr(const YAML::Node& n){
	std::string s;
	n >> s;
	return tocstring(s);
}

#endif /* YAML_UTIL_H_ */
