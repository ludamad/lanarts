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
#include <typeinfo>

ItemInst::~ItemInst() {
}

void ItemInst::step(GameState *gs) {
}

void ItemInst::draw(GameState *gs) {
	GameView& view = gs->window_view();

	ItemEntry& itemd = game_item_data[type];
	GLimage& img = game_sprite_data[itemd.sprite_number].img;

	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;
	int x_inview = xx - view.x, y_inview = yy - view.y;
	gl_draw_image(&img, x_inview, y_inview);
	if (itemd.stackable && quantity > 1) {
		gl_printf(gs->primary_font(), Colour(255,255,255), x_inview+1, y_inview+1, "%d", quantity);
	}
}

void ItemInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ItemInst*)inst = *this;
}

ItemInst *ItemInst::clone() const {
	return new ItemInst(*this);
}

