/*
 * GameHud.h
 *
 *  Created on: 2011-11-06
 *      Author: 100400530
 */

#ifndef GAMEHUD_H_
#define GAMEHUD_H_

#include "../display/display.h"
#include "../display/GLImage.h"
#include "../util/draw_util.h"
#include "objects/GameInst.h"
#include "GameInstSet.h"

#define INVENTORY_POSITION 296


/* Component of GameState that draws statistic overview */
class GameHud {
public:
	/* Draw the game statistics overlay */
	void draw(GameState* gs);
	GameHud(int x, int y, int width, int height);
	~GameHud();

	/* Width of overlay */
	int width() { return _width; }
	/* Height of overlay */
	int height() { return _height; }
private:
	/* Helper method for drawing mini-map based on game information */
	void draw_minimap(GameState* gs, int subx, int suby);

	int x, y, _width, _height;
	Colour bg_colour;
	char* minimap_arr;
	GLImage minimap_buff;
};



#endif /* GAMEHUD_H_ */
