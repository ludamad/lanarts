
#ifndef GAMESETTINGS_H_
#define GAMESETTINGS_H_
#include <string>

struct GameSettings {
	enum connection_type {
		NONE,
		CLIENT,
		HOST
	};

	/*Multiplayer settings*/
	std::string ip;
	int port;
	connection_type conntype;

	/*Window settings*/
	std::string font;
	bool fullscreen;
	int view_width, view_height;

	/*Gameplay settings*/
	bool regen_on_death;
	int classn;

	GameSettings(){
		//sets defaults
		fullscreen = false;
		regen_on_death = false;

		classn = 1;
		view_width = 960;
		view_height = 720;

		font = "res/arial.ttf";

		port = 0;
		conntype = NONE;

	}
};


#endif /* GAMESETTINGS_H_ */
