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
	std::string font;
	bool fullscreen;
	int view_width, view_height;
	int steps_per_draw;
	float time_per_step;

	/*Debug options*/
	bool draw_diagnostics, verbose_output;
	bool invincible;

	/*Replay settings, can be set in menu*/
	std::string savereplay_file, loadreplay_file;

	/*Permanent gameplay settings*/
	bool regen_on_death;
	class_id classn;

	/*Gameplay settings changeable in game*/
	bool autouse_health_potions, autouse_mana_potions;

	GameSettings() {
		//sets defaults
		fullscreen = false;
		regen_on_death = false;

		classn = 1;
		view_width = 960;
		view_height = 720;
		steps_per_draw = 1;
		time_per_step = 12;

		font = "res/MateSC-Regular.ttf";
		invincible = false;
		draw_diagnostics = false;

		port = 0;
		conntype = NONE;
		network_debug_mode = false;
		verbose_output = false;

		autouse_health_potions = true;
		autouse_mana_potions = true;
	}
};

#endif /* GAMESETTINGS_H_ */
