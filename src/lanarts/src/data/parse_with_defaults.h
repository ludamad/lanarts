/*
 * parse_with_defaults.h:
 *  Utilities for parsing a yaml map or list with a 'default:' or 'defaults:' node.
 *  The 'default' or 'defaults' node are merged into the end results.
 */

#ifndef PARSE_WITH_DEFAULTS_H_
#define PARSE_WITH_DEFAULTS_H_

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <yaml-cpp/yaml.h>

#include "parse.h"

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


#endif /* PARSE_WITH_DEFAULTS_H_ */
