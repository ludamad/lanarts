/*
 * parse.h:
 *  Simple YAML convenience routines
 */

#ifndef PARSE_H_
#define PARSE_H_

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

// The data loaders extract values with 'node >> value', the style of the yaml-cpp 0.3 API they
// were written against. These keep that working on top of Node::as<T>(); lanarts types get their
// own (non-template) overloads, which take precedence.
template<typename T>
inline void operator>>(const YAML::Node& node, T& value) {
	value = node.as<T>();
}

template<typename T>
inline void operator>>(const YAML::Node& node, std::vector<T>& values) {
	values.clear();
	values.resize(node.size());
	for (size_t i = 0; i < node.size(); i++) {
		node[i] >> values[i];
	}
}

bool yaml_has_node(const YAML::Node& node, const char* child);

std::string parse_str(const YAML::Node& node);
int parse_int(const YAML::Node& node);

template<typename T>
inline T parse(const YAML::Node& node) {
	T val;
	node >> val;
	return val;
}

template<class T>
inline T parse_optional(const YAML::Node& n, const char* key, const T& dflt =
		T()) {
	if (!yaml_has_node(n, key)) {
		return dflt;
	}
	T ret;
	n[key] >> ret;
	return ret;
}

#endif /* PARSE_H_ */
