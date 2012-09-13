#ifndef GAMESETTINGS_H_
#define GAMESETTINGS_H_
#include <string>

#include "../lanarts_defines.h"

struct GameSettings {
	enum connection_type {
		NONE, CLIENT, SERVER
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

	/* Speed related options */
	int steps_per_draw;
	float time_per_step;
	int frame_action_repeat;

	/*Debug options*/
	bool draw_diagnostics, verbose_output;
	bool invincible;
	bool keep_event_log;
	std::string comparison_event_log;

	/*Replay settings, can be set in menu*/
	std::string savereplay_file, loadreplay_file;

	/*Permanent gameplay settings*/
	bool regen_on_death;
	class_id classtype;

	/*Gameplay settings changeable in game*/
	bool autouse_health_potions, autouse_mana_potions;

	GameSettings() {
		//sets defaults
		fullscreen = false;
		regen_on_death = true;

		classtype = 1;
		view_width = 960;
		view_height = 720;
		steps_per_draw = 1;
		time_per_step = 12;
		frame_action_repeat = 0;

		font = "res/fonts/Gudea-Regular.ttf";
		menu_font = "res/fonts/MateSC-Regular.ttf";

		invincible = false;
		draw_diagnostics = false;

		port = 0;
		conntype = NONE;
		network_debug_mode = false;
		verbose_output = false;

		autouse_health_potions = true;
		autouse_mana_potions = true;

		keep_event_log = false;
	}

	bool saving_to_action_file() {
		return !savereplay_file.empty();
	}
	bool loading_from_action_file() {
		return !loadreplay_file.empty();
	}
};

#endif /* GAMESETTINGS_H_ */
