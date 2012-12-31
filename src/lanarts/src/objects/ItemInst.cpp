/*
 * ItemInst.cpp:
 *  Represents an item on the floor
 */

#include <typeinfo>

#include <lcommon/mathutil.h>

#include <ldraw/draw.h>
#include <ldraw/Font.h>
#include <ldraw/DrawOptions.h>
#include <ldraw/colour_constants.h>

#include "display/SpriteEntry.h"
#include "gamestate/GameState.h"
#include "gamestate/GameView.h"
#include "stats/items/ItemEntry.h"

#include "ItemInst.h"
#include "collision_filters.h"

ItemInst::~ItemInst() {
}

static bool same_item_colfilter(GameInst* self, GameInst* other) {
	LANARTS_ASSERT(dynamic_cast<ItemInst*>(self));
	ItemInst* other_item = dynamic_cast<ItemInst*>(other);
	return other_item
			&& ((ItemInst*)self)->item_type().is_same_item(
					other_item->item_type());
}
void ItemInst::step(GameState *gs) {
	GameInst* other_item = NULL;
	ItemEntry& ientry = item.item_entry();
	if (ientry.stackable
			&& gs->object_radius_test(this, &other_item, 1,
					same_item_colfilter)) {
		ItemInst* oinst = (ItemInst*)other_item;
		if (oinst->item.is_same_item(item) && id < oinst->id) {
			gs->remove_instance(oinst);
			item.amount += oinst->item_quantity();
		}
	}
}

void ItemInst::draw(GameState* gs) {
	GameView& view = gs->view();

	ItemEntry& ientry = item.item_entry();
	SpriteEntry& spr = ientry.item_sprite_entry();

	int w = spr.width(), h = spr.height();
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!was_seen && !gs->object_visible_test(this))
		return;

	was_seen = true;

	Pos p = round_to_multiple(pos(), TILE_SIZE);

	ldraw::draw_rectangle_outline(COL_WHITE.alpha(45),
			on_screen(gs, BBox(p.x + 1, p.y + 1, p.x + TILE_SIZE, p.y + TILE_SIZE)));

	gl_draw_sprite(view, ientry.item_sprite, xx, yy, 0, 0, gs->frame());

	if (ientry.stackable && item_quantity() > 1) {
		gs->font().drawf(COL_WHITE, Pos(xx - view.x + 1, yy - view.y + 1), "%d",
				item_quantity());
	}

}

void ItemInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ItemInst*)inst = *this;
}

ItemInst* ItemInst::clone() const {
	return new ItemInst(*this);
}

void ItemInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	item.serialize(serializer);
	serializer.write(dropped_by);
	serializer.write(pickup_by_dropper);
}

void ItemInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	item.deserialize(serializer);
	serializer.read(dropped_by);
	serializer.read(pickup_by_dropper);
}

