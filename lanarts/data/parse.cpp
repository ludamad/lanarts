/*
 * parse.cpp:
 *  Simple YAML convenience routines
 */

#include "parse.h"
#include <yaml-cpp/yaml.h>

const YAML::Node* yaml_find_node(const YAML::Node& node, const char* child) {
	return node.FindValue(child);
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
