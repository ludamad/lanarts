
#ifndef GAMESETTINGS_H_
#define GAMESETTINGS_H_

struct GameSettings {
	bool fullscreen;
	bool regen_on_death;

	int view_width, view_height;
	GameSettings(){
		//sets defaults
		fullscreen = false;
		regen_on_death = false;

		view_width = 960;
		view_height = 720;
	}
};


#endif /* GAMESETTINGS_H_ */
