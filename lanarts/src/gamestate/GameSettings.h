/*
 * Contains configuration for lanarts
 * All data relevant to chosen game mode, class, configuration options etc is stored here.
 */

#ifndef GAMESETTINGS_H_
#define GAMESETTINGS_H_
#include <string>

#include "lanarts_defines.h"

class SerializeBuffer;

struct GameSettings {
	enum connection_type {
		NONE = 0, CLIENT = 1, SERVER = 2
	};

	/*Multiplayer settings*/
	std::string ip, username;
	int port;
	connection_type conntype;
	bool network_debug_mode;

	/*Draw settings*/
	std::string font, menu_font;
	bool fullscreen;
	int view_width, view_height;

	/* Performance related options */
	int steps_per_draw;
	float time_per_step;
	int frame_action_repeat;
	bool free_memory_while_idle;

	/*Debug options*/
	bool draw_diagnostics, verbose_output;
	bool invincible;
	bool keep_event_log;
	std::string comparison_event_log;

	/*Replay settings, can be set in menu*/
	std::string savereplay_file, loadreplay_file;

	/*Permanent gameplay settings*/
	bool regen_on_death;
	class_id class_type;

	/*Gameplay settings changeable in game*/
	bool autouse_health_potions, autouse_mana_potions;

	GameSettings() {
		//sets defaults
		username = "User";

		fullscreen = false;
		regen_on_death = true;

		class_type = -1;
		view_width = 960;
		view_height = 720;
		steps_per_draw = 1;
		time_per_step = 14;
		frame_action_repeat = 0;
		free_memory_while_idle = false;

		font = "res/fonts/Gudea-Regular.ttf";
		menu_font = "res/fonts/Gudea-Regular.ttf";

		invincible = false;
		draw_diagnostics = false;

		ip = "localhost";
		port = 6112;
		conntype = NONE;
		network_debug_mode = false;
		verbose_output = false;

		autouse_health_potions = true;
		autouse_mana_potions = true;

		keep_event_log = false;
	}

	void serialize_gameplay_settings(SerializeBuffer& serializer) const;
	void deserialize_gameplay_settings(SerializeBuffer& serializer);

	bool saving_to_action_file() {
		return !savereplay_file.empty();
	}
	bool loading_from_action_file() {
		return !loadreplay_file.empty();
	}
};

#endif /* GAMESETTINGS_H_ */
