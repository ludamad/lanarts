/*
 * load_settings_data.cpp:
 *  Load configuration data.
 */

#include <fstream>

#include "../../data/game_data.h"
#include <yaml-cpp/yaml.h>
#include "../../data/yaml_util.h"

using namespace std;


GameSettings load_settings_data(const char* filename) {

	fstream file(filename, fstream::in | fstream::binary);

	GameSettings ret;
	if (file) {
		try {
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);

			optional_set(root, "font", ret.font);
			optional_set(root, "fullscreen", ret.fullscreen);
			optional_set(root, "regen_level_on_death", ret.regen_on_death);
			optional_set(root, "view_width", ret.view_width);
			optional_set(root, "view_height", ret.view_height);
			optional_set(root, "ip", ret.ip);
			optional_set(root, "port", ret.port);
			optional_set(root, "steps_per_draw", ret.steps_per_draw);
			if (ret.steps_per_draw < 1)
				ret.steps_per_draw = 1;
			optional_set(root, "invincible", ret.invincible);
			optional_set(root, "time_per_step", ret.time_per_step);
			optional_set(root, "draw_diagnostics", ret.draw_diagnostics);
			optional_set(root, "username", ret.username);
			optional_set(root, "use_stop_actions", ret.stop_controls);
			optional_set(root, "network_debug_mode", ret.network_debug_mode);
			optional_set(root, "savereplay_file", ret.savereplay_file);
			optional_set(root, "loadreplay_file", ret.loadreplay_file);
			optional_set(root, "verbose_output", ret.verbose_output);

			if (yaml_has_node(root, "connection_type")) {
				std::string connname;
				root["connection_type"] >> connname;
				if (connname == "none") {
					ret.conntype = GameSettings::NONE;
				} else if (connname == "client") {
					ret.conntype = GameSettings::CLIENT;
				} else if (connname == "host") {
					ret.conntype = GameSettings::SERVER;
				}
			}

			if (yaml_has_node(root, "class")) {
				std::string classname;
				root["class"] >> classname;
				if (!game_class_data.empty())
					ret.classn = get_class_by_name(classname.c_str());
			}
		} catch (const YAML::Exception& parse) {
			printf("Settings Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}

	return ret;
}
