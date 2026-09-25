/*
 * parse.cpp:
 *  Simple YAML convenience routines
 */

#include "parse.h"

bool yaml_has_node(const YAML::Node& node, const char* child) {
	return node.IsMap() && node[child].IsDefined();
}

int parse_int(const YAML::Node& n) {
	return n.as<int>();
}

std::string parse_str(const YAML::Node& n) {
	return n.as<std::string>();
}
