/*
 * StoreContent.cpp:
 *  Represents an interactive view of a store for the side bar
 */

#include "../../display/colour_constants.h"
#include "../../display/display.h"

#include "../../display/sprite_data.h"

#include "../../gamestate/GameState.h"
#include "../../objects/player/PlayerInst.h"

#include "../../objects/store/StoreInst.h"

#include "../../stats/items/ItemEntry.h"
#include "../console_description_draw.h"

#include "StoreContent.h"

const int STORE_SLOT_H = TILE_SIZE * 1.25;

//Draw in bottom-right of slot
static void draw_slot_cost(GameState* gs, money_t cost, int x, int y) {
	Dim dim = gl_text_dimensions(gs->primary_font(), "%dg", cost);
	gl_printf_x_centered(gs->primary_font(), COL_PALE_YELLOW, x + TILE_SIZE / 2,
			y + STORE_SLOT_H - dim.h - 2, "%d", cost);
}

static void draw_store_inventory_slot(GameState* gs, StoreItemSlot& itemslot,
		int x, int y) {
	if (!itemslot.empty()) {
		ItemEntry& ientry = itemslot.item.item_entry();
		GLimage& itemimg = ientry.item_image();
		gl_draw_image(itemimg, x, y);
		int amnt = itemslot.item.amount;
		if (ientry.stackable && amnt > 1) {
			gl_printf(gs->primary_font(), Colour(255, 255, 255), x + 1, y + 1,
					"%d", amnt);
		}
		draw_slot_cost(gs, itemslot.cost, x, y);
	}
}

static void draw_item_cost(GameState* gs, const BBox& bbox, int cost) {
	PlayerInst* player = gs->local_player();
	Colour col = COL_PALE_GREEN;
	if (player->gold() < cost) {
		col = COL_PALE_RED;
	}
	gl_printf_centered(gs->primary_font(), col, bbox.center_x(),
			bbox.y1 - TILE_SIZE / 2, "Cost: %dg", cost);
}
static void draw_store_inventory(GameState* gs, StoreInventory& inv,
		const BBox& bbox, int min_slot, int max_slot, int slot_selected = -1) {
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int slot = min_slot;
	int lastslot = inv.last_filled_slot();

	for (int y = bbox.y1; y + STORE_SLOT_H <= bbox.y2; y += STORE_SLOT_H) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (slot >= max_slot || slot >= inv.max_size())
				break;

			StoreItemSlot& itemslot = inv.get(slot);

			if (slot != slot_selected)
				draw_store_inventory_slot(gs, itemslot, x, y);
			BBox slotbox(x, y, x + TILE_SIZE, y + STORE_SLOT_H);
			Colour outline(COL_UNFILLED_OUTLINE);
			if (!itemslot.empty() && slot != slot_selected) {
				outline = COL_FILLED_OUTLINE;
				if (slotbox.contains(mx, my)) {
					outline = COL_PALE_YELLOW;
					draw_console_item_description(gs, itemslot.item);
					draw_item_cost(gs, bbox, itemslot.cost);
				}
			}

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

static int get_itemslotn(StoreInventory& inv, const BBox& bbox, int mx,
		int my) {
	if (!bbox.contains(mx, my)) {
		return -1;
	}

	int posx = (mx - bbox.x1) / TILE_SIZE;
	int posy = (my - bbox.y1) / STORE_SLOT_H;
	int slot = 5 * posy + posx;

	if (slot < 0 || slot >= inv.max_size())
		return -1;

	return slot;
}

bool StoreContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();
	StoreInventory& inv = store_inventory();
	int mx = gs->mouse_x(), my = gs->mouse_y();
	bool within_inventory = bbox.contains(mx, my);

	/* Buy an item (left click) */
	if (gs->mouse_left_click() && within_inventory) {

		int slot = get_itemslotn(inv, bbox, mx, my);
		if (slot >= 0 && slot < INVENTORY_SIZE && !inv.get(slot).empty()) {
			if (p->gold() >= inv.get(slot).cost) {
				queued_actions.push_back(
						game_action(gs, p, GameAction::PURCHASE_FROM_STORE,
								store_object->id, NONE, NONE, slot));
			} else {
				gs->game_chat().add_message("You cannot afford it!",
						COL_PALE_RED);
			}
			return true;
		}
	}
	return false;
}

