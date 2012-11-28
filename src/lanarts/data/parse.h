/*
 * parse.h:
 *  Simple YAML convenience routines
 */

#ifndef PARSE_H_
#define PARSE_H_

#include <string>

namespace YAML {
	class Node;
}

bool yaml_has_node(const YAML::Node& node, const char* child);
const YAML::Node* yaml_find_node(const YAML::Node& node, const char* child);

std::string parse_str(const YAML::Node& node);
int parse_int(const YAML::Node& node);

template <typename T>
inline T parse(const YAML::Node& node) {
	T val;
	node >> val;
	return val;
}

template<class T>
inline T parse_optional(const YAML::Node& n, const char* key, const T& dflt = T()) {
	T ret;
	if (const YAML::Node* sptr = yaml_find_node(n, key)) {
		*sptr >> ret;
	} else
		return dflt;
	return ret;
}

#endif /* PARSE_H_ */
