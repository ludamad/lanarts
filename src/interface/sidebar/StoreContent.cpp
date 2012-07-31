/*
 * StoreContent.cpp:
 *  Represents an interactive view of a store for the side bar
 */

#include "../../stats/item_data.h"
#include "../../display/sprite_data.h"

#include "../../display/display.h"

#include "../../display/colour_constants.h"
#include "../console_description_draw.h"

#include "../../gamestate/GameState.h"

#include "../../objects/store/StoreInst.h"
#include "../../objects/player/PlayerInst.h"

#include "StoreContent.h"

static void draw_store_inventory_slot(GameState* gs, StoreItemSlot& itemslot,
		int x, int y) {
	if (itemslot.amount > 0) {
		ItemEntry& ientry = itemslot.item.item_entry();
		GLimage& itemimg = game_sprite_data[ientry.sprite].img();
		gl_draw_image(itemimg, x, y);
		if (ientry.stackable) {
			gl_printf(gs->primary_font(), Colour(255, 255, 255), x + 1, y + 1,
					"%d", itemslot.amount);
		}
	}
}

static void draw_store_inventory(GameState* gs, StoreInventory& inv,
		const BBox& bbox, int min_slot, int max_slot, int slot_selected = -1) {
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int slot = min_slot;
	int lastslot = inv.last_filled_slot();
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (slot >= max_slot || slot >= inv.max_size())
				break;

			StoreItemSlot& itemslot = inv.get(slot);

			BBox slotbox(x, y, x + TILE_SIZE, y + TILE_SIZE);
			Colour outline(COL_UNFILLED_OUTLINE);
			if (itemslot.amount > 0 && slot != slot_selected) {
				outline = COL_FILLED_OUTLINE;
				if (slotbox.contains(mx, my)) {
					outline = COL_PALE_YELLOW;
					draw_console_item_description(gs, itemslot.item);
				}
			}

			if (slot != slot_selected)
				draw_store_inventory_slot(gs, itemslot, x, y);
			//draw rectangle over item edges
			gl_draw_rectangle_outline(slotbox, outline);

			slot++;
		}
	}

	if (slot_selected != -1) {
		draw_store_inventory_slot(gs, inv.get(slot_selected),
				gs->mouse_x() - TILE_SIZE / 2, gs->mouse_y() - TILE_SIZE / 2);
	}
}

void StoreContent::draw(GameState* gs) const {
	StoreInventory& inv = store_inventory();
	draw_store_inventory(gs, inv, bbox, 0, 40);
//	inv.
}

StoreInventory& StoreContent::store_inventory() const {
	return store_object->inventory();
}

bool StoreContent::handle_io(GameState* gs, ActionQueue & queued_actions) {

	return true;
}

