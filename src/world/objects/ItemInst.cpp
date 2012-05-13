/*
 * ItemInst.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#include "ItemInst.h"
#include "../GameState.h"
#include "../../data/item_data.h"
#include "../../data/sprite_data.h"


ItemInst::~ItemInst() {
}

void ItemInst::step(GameState *gs){
}



void ItemInst::draw(GameState *gs){
	GameView& view = gs->window_view();

	ItemType& itemd = game_item_data[type];
	GLimage& img = game_sprite_data[itemd.sprite_number].img;

	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	gl_draw_image(&img, xx - view.x, yy - view.y);
}



