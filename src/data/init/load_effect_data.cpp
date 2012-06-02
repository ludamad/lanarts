#include <fstream>
#include <string>

#include <yaml-cpp/yaml.h>

#include "../../gamestats/Stats.h"

#include "../effect_data.h"
#include "../game_data.h"

#include "yaml_util.h"

using namespace std;

EffectEntry parse_effect(const YAML::Node& n) {
	return EffectEntry(parse_cstr(n["name"]),
			parse_defaulted(n, "stat_func", std::string()));
}

LuaValue load_effect_data(lua_State* L, const char* filename) {
	LuaValue ret;
	fstream file(filename, fstream::in | fstream::binary);

	if (file) {
		try {
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);
			game_effect_data.clear();
			const YAML::Node& effects = root["effects"];
			for (int i = 0; i < effects.size(); i++) {
				game_effect_data.push_back(parse_effect(effects[i]));
			}
		} catch (const YAML::Exception& parse) {
			printf("Effects Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
		file.close();
	}
	return ret;
}
