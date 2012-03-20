/*
 * ItemInst.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#include "ItemInst.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"


ItemInst::~ItemInst() {
}

void ItemInst::step(GameState *gs){
}



void ItemInst::draw(GameState *gs){
	GameView& view = gs->window_view();
	GLImage& img = game_sprite_data[SPR_GOLD].img;

	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	//if (!view.within_view(xx, yy, w, h))
	//	return;
	//if (!gs->object_visible_test(this))
	//	return;

	image_display(&img, xx - view.x, yy - view.y);
}



