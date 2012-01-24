/*
 * GameState.h
 *
 *  Created on: 2011-09-29
 *      Author: 100397561
 */

#ifndef GAMESTATE_H_
#define GAMESTATE_H_
#include <vector>
#include "objects/GameInst.h"
#include "GameInstSet.h"
#include "GameTiles.h"
#include "GameView.h"
#include "GameHud.h"
#include "../util/font_util.h"
#include <SDL.h>

class GameState {
public:
	GameState(int width, int height, int vieww = 640, int viewh = 480, int hudw = 160);
	~GameState();

	/* Each frame is divided into a step event, and a draw event */
	void draw();
	bool step();

	/* INSTANCE HANDLING METHODS */
	GameInst* get_instance(obj_id id);
	obj_id add_instance(GameInst* inst);
	void remove_instance(GameInst* inst);

	/* COLLISION METHODS */
	bool tile_radius_test(int x, int y, int rad);
	/* Check whether intersects a solid object */
	bool solid_radius_test(int x, int y, int rad);
	bool tile_line_test(int x, int y, int w, int h);

	/* GameState components */
	GameView& window_view() { return view; }
	GameTiles& tile_grid() { return tiles; }

	/* Default font for most text rendering */
	const font_data& primary_font(){ return pfont; }

	/* Mouse state information */
	int mouse_x(){ return mousex; }
	int mouse_y(){ return mousey; }
	bool mouse_left_click() { return left_click; }

	/* Key state query information */
	int key_press_state(int keyval);

	/* Object identifier for the player */
	obj_id& player_id() { return player; }

	/* Getters for world size */
	int width(){ return world_width; }
	int height(){ return world_height; }

	void serialize(FILE* file);

private:
	void restart();
	int world_width, world_height;
	int handle_event(SDL_Event* event);
	int frame_n;
	obj_id player;
    GameTiles tiles;
    GameInstSet inst_set;
    GameHud hud;
	GameView view;
	font_data pfont;

	char key_states[SDLK_LAST];
	int mousex, mousey;
	bool left_click, right_click;
};

#endif /* GAMESTATE_H_ */
