/*
 * GameState.h
 *
 *  Created on: 2011-09-29
 *      Author: 100397561
 */

#ifndef GAMESTATE_H_
#define GAMESTATE_H_
#include <vector>
#include <SDL.h>
#include "objects/GameInst.h"
#include "controllers/MonsterController.h"
#include "controllers/PlayerController.h"
#include "GameLevelState.h"
#include "GameView.h"
#include "GameHud.h"
#include "../util/font_util.h"
#include "../procedural/mtwist.h"
#include "../fov/fov.h"

class GameState {
public:

	GameState(int width, int height, int vieww = 640, int viewh = 480,
			int hudw = 160);
	~GameState();

	/* Each frame is divided into a step event, and a draw event */
	void draw();
	bool step();

	/* INSTANCE HANDLING METHODS */
	GameInst* get_instance(obj_id id);
	obj_id add_instance(GameInst* inst);
	void remove_instance(GameInst* inst);

	/* COLLISION METHODS */
	bool tile_radius_test(int x, int y, int rad, bool issolid = true,
			int ttype = -1, Pos* hitloc = NULL);
	/* Check whether intersects a solid object */
	bool solid_radius_test(int x, int y, int rad);
	bool tile_line_test(int x, int y, int w, int h);
	int object_radius_test(GameInst* obj, GameInst** objs = NULL, int obj_cap =
			0, col_filterf f = NULL, int x = -1, int y = -1, int radius = -1);
	bool solid_test(GameInst* obj, GameInst** objs = NULL, int obj_cap = 0,
			col_filterf f = NULL, int x = -1, int y = -1, int radius = -1) {
		int lx = (x == -1 ? obj->x : x), ly = (y == -1 ? obj->y : y);
		return tile_radius_test(lx, ly, obj->radius)
				|| object_radius_test(obj, objs, obj_cap, f, x, y, radius);
	}
	bool object_visible_test(GameInst* obj);

	/* GameState components */
	GameView& window_view() {
		return view;
	}
	GameTiles& tile_grid() {
		return lvl->tiles;
	}
	/* Game object central controllers */
	MonsterController& monster_controller() {
		return lvl->mc;
	}
	PlayerController& player_controller() {
		return lvl->pc;
	}

	/* Default font for most text rendering */
	const font_data& primary_font() {
		return pfont;
	}
	//Finds the nearest object to 'obj' with some condition true
	//Takes on the order of (max_radius*2/96)^2 time
	GameInst* nearest_object(GameInst* obj, int max_radius,
			col_filterf f = NULL);

	/* Mouse state information */
	int mouse_x() {
		return mousex;
	}
	int mouse_y() {
		return mousey;
	}
	bool mouse_left_click() {
		return mouse_leftclick;
	}
	bool mouse_right_click() {
		return mouse_rightclick;
	}
	bool mouse_left_down() {
		return mouse_leftdown;
	}
	bool mouse_right_down() {
		return mouse_rightdown;
	}

	int frame() {
		return frame_n;
	}
	/* Object identifier for the player */
	obj_id local_playerid() {
		return lvl->pc.local_playerid();
	}

	/* Key state query information */
	int key_down_state(int keyval);
	int key_press_state(int keyval);

	/* Getters for world size */
	int width() {
		return world_width;
	}
	int height() {
		return world_height;
	}
	int& branch_level() {
		return level_number;
	}
	GameLevelState* level() {
		return lvl;
	}

	void serialize(FILE* file);

	MTwist& rng() {
		return mtwist;
	}
	void set_generate_flag() {
		gennextstep = true;
	}

	void reset_level();
	void set_level(int levelnum, bool reset);

private:
	std::vector<GameLevelState*> level_states;

	void restart();
	int handle_event(SDL_Event* event);

	//Game bounds
	int world_width, world_height;

	//Game location information
	int level_number;

	int frame_n;
	bool gennextstep;

	GameLevelState* lvl;
	GameHud hud;
	GameView view;

	//Mersenne twister random number generator state
	MTwist mtwist;

	//Primary font data
	font_data pfont;

	//Key/mouse state information
	char key_down_states[SDLK_LAST];
	char key_press_states[SDLK_LAST];
	int mousex, mousey;
	bool mouse_leftdown, mouse_rightdown;
	bool mouse_leftclick, mouse_rightclick;
};

#endif /* GAMESTATE_H_ */
