/*
 * load_settings_data.cpp:
 *  Load configuration data.
 */

#include <fstream>

#include "data/game_data.h"
#include "stats/ClassEntry.h"
#include <yaml-cpp/yaml.h>
#include "data/yaml_util.h"

using namespace std;

template<typename T>
static void save_yaml_attr(fstream& file, const char* key, const T& value) {
	file << key << ": " << value << '\n';
}

// Saves certain settings to a file
void save_settings_data(GameSettings& settings, const char* filename) {
	fstream file(filename, fstream::out);

	if (file) {
		file << "# See settings.yaml for details\n";
		save_yaml_attr(file, "ip", settings.ip);
		save_yaml_attr(file, "username", settings.username);
		save_yaml_attr(file, "steps_per_draw", settings.steps_per_draw);
		save_yaml_attr(file, "time_per_step", settings.time_per_step);
		save_yaml_attr(file, "frame_action_repeat", settings.frame_action_repeat);

		save_yaml_attr(file, "class", settings.class_type);

		save_yaml_attr(file, "port", settings.port);
		save_yaml_attr(file, "regen_level_on_death",
				settings.regen_on_death ? "yes" : "no");
		save_yaml_attr(file, "autouse_health_potions",
				settings.autouse_health_potions ? "yes" : "no");
		save_yaml_attr(file, "autouse_mana_potions",
				settings.autouse_mana_potions ? "yes" : "no");
		const char* connection_strings[] = { "none", "client", "server" };
		save_yaml_attr(file, "connection_type",
				connection_strings[settings.conntype]);
	}
}
bool load_settings_data(GameSettings& settings, const char* filename) {

	fstream file(filename, fstream::in);

	if (file) {
		try {
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);

			optional_set(root, "font", settings.font);
			optional_set(root, "menu_font", settings.menu_font);
			optional_set(root, "fullscreen", settings.fullscreen);
			optional_set(root, "regen_level_on_death", settings.regen_on_death);
			optional_set(root, "view_width", settings.view_width);
			optional_set(root, "view_height", settings.view_height);
			optional_set(root, "ip", settings.ip);
			optional_set(root, "port", settings.port);
			optional_set(root, "lobby_server_url", settings.lobby_server_url);
			optional_set(root, "steps_per_draw", settings.steps_per_draw);
			if (settings.steps_per_draw < 1)
				settings.steps_per_draw = 1;
			optional_set(root, "frame_action_repeat",
					settings.frame_action_repeat);
			optional_set(root, "free_memory_while_idle",
					settings.free_memory_while_idle);
			if (settings.frame_action_repeat < 0)
				settings.frame_action_repeat = 0;
			optional_set(root, "invincible", settings.invincible);
			optional_set(root, "time_per_step", settings.time_per_step);
			optional_set(root, "draw_diagnostics", settings.draw_diagnostics);
			optional_set(root, "username", settings.username);
			optional_set(root, "network_debug_mode",
					settings.network_debug_mode);
			optional_set(root, "savereplay_file", settings.savereplay_file);
			optional_set(root, "loadreplay_file", settings.loadreplay_file);
			optional_set(root, "verbose_output", settings.verbose_output);
			optional_set(root, "autouse_health_potions",
					settings.autouse_health_potions);
			optional_set(root, "autouse_mana_potions",
					settings.autouse_mana_potions);
			optional_set(root, "keep_event_log", settings.keep_event_log);
			optional_set(root, "comparison_event_log",
					settings.comparison_event_log);

			if (yaml_has_node(root, "connection_type")) {
				std::string connname;
				root["connection_type"] >> connname;
				if (connname == "none") {
					settings.conntype = GameSettings::NONE;
				} else if (connname == "client") {
					settings.conntype = GameSettings::CLIENT;
				} else if (connname == "server") {
					settings.conntype = GameSettings::SERVER;
				}
			}

			if (yaml_has_node(root, "class")) {
				root["class"] >> settings.class_type;
			}
		} catch (const YAML::Exception& parse) {
			printf("Settings Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}

		return true; /* File did exist */
	}
	return false; /* File did not exist */
}
