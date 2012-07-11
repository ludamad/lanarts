/*
 * ItemInst.cpp:
 *  Represents an item on the floor
 */

#include "ItemInst.h"
#include "../GameState.h"
#include "../../util/world/collision_util.h"
#include "../../data/item_data.h"
#include "../../data/sprite_data.h"
#include <typeinfo>

ItemInst::~ItemInst() {
}

static bool same_item_colfilter(GameInst* self, GameInst* other) {
	LANARTS_ASSERT(dynamic_cast<ItemInst*>(self));
	ItemInst* other_item = dynamic_cast<ItemInst*>(other);
	return other_item
			&& ((ItemInst*) self)->item_type() == other_item->item_type();
}
void ItemInst::step(GameState *gs) {
	GameInst* other_item = NULL;
	ItemEntry& ientry = item.item_entry();
	if (ientry.stackable
			&& gs->object_radius_test(this, &other_item, 1, same_item_colfilter)) {
		ItemInst* oinst = (ItemInst*) other_item;
		if (oinst->item == item && id < oinst->id) {
			gs->remove_instance(oinst);
			quantity += oinst->item_quantity();
		}
	}
}

void ItemInst::draw(GameState* gs) {
	GameView& view = gs->window_view();

	ItemEntry& ientry = item.item_entry();
	SpriteEntry& spr = game_sprite_data.at(ientry.sprite);

	int w = spr.width(), h = spr.height();
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	gl_draw_sprite(view, ientry.sprite, xx, yy, 0, 0, gs->frame());
	if (ientry.stackable && quantity > 1) {
		gl_printf(gs->primary_font(), Colour(255, 255, 255), xx - view.x + 1,
				yy - view.y + 1, "%d", quantity);
	}
}

void ItemInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ItemInst*) inst = *this;
}

ItemInst *ItemInst::clone() const {
	return new ItemInst(*this);
}

