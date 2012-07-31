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

	/*Replay settings, can be set in menu*/
	std::string savereplay_file, loadreplay_file;

	/*Gameplay settings*/
	bool regen_on_death;
	bool invincible;
	class_id classn;

	/*Control settings*/
	bool stop_controls;

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

		stop_controls = true;

		port = 0;
		conntype = NONE;
		network_debug_mode = false;
		verbose_output = false;
	}
};

#endif /* GAMESETTINGS_H_ */