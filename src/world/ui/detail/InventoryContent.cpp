/*
 * InventoryContent.cpp:
 *  Represents an interactive view of the inventory for the side bar
 */

#include "../../../data/item_data.h"
#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

#include "../../../util/colour_constants.h"
#include "../../../util/content_draw_util.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "InventoryContent.h"

static void draw_player_inventory_slot(GameState* gs, ItemSlot& itemslot, int x,
		int y) {
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

static void draw_player_inventory(GameState* gs, Inventory& inv,
		const BBox& bbox, int min_slot, int max_slot, int slot_selected = -1) {
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int slot = min_slot;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (slot >= max_slot)
				break;

			ItemSlot& itemslot = inv.get(slot);

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
				draw_player_inventory_slot(gs, itemslot, x, y);
			//draw rectangle over item edges
			gl_draw_rectangle_outline(slotbox, outline);

			slot++;
		}
	}

	if (slot_selected != -1) {
		draw_player_inventory_slot(gs, inv.get(slot_selected),
				gs->mouse_x() - TILE_SIZE / 2, gs->mouse_y() - TILE_SIZE / 2);
	}
}

static int get_itemslotn(Inventory& inv, const BBox& bbox, int mx, int my) {
	if (!bbox.contains(mx, my)) {
		return -1;
	}

	int posx = (mx - bbox.x1) / TILE_SIZE;
	int posy = (my - bbox.y1) / TILE_SIZE;
	int slot = 5 * posy + posx;

	if (slot < 0 || slot >= inv.max_size())
		return -1;

	return slot;
}

const int ITEMS_PER_PAGE = 40;

void InventoryContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	Inventory& inv = p->inventory();
	int min_item = ITEMS_PER_PAGE * page_number, max_item = min_item
			+ ITEMS_PER_PAGE;
	draw_player_inventory(gs, inv, bbox, min_item, max_item, slot_selected);
}

int InventoryContent::amount_of_pages(GameState* gs) {
	PlayerInst* p = gs->local_player();

	int items_n = p->inventory().size();
	/* Add ITEMS_PER_PAGE - 1 so that 0 spells need 0 pages, 1 spell needs 1 page, etc*/
	int item_pages = (items_n + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;

	return item_pages;
}

bool InventoryContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();
	Inventory inv = p->inventory();
	int mx = gs->mouse_x(), my = gs->mouse_y();
	bool within_inventory = bbox.contains(mx, my);

	/* Use an item */
	if (gs->mouse_left_click() && within_inventory) {

		int slot = get_itemslotn(inv, bbox, mx, my);
		if (slot >= 0 && slot < INVENTORY_SIZE && inv.get(slot).amount > 0) {
			queued_actions.push_back(
					game_action(gs, p, GameAction::USE_ITEM, slot, p->x, p->y));
			return true;
		}
	}

	/* Start dragging an item */
	if (gs->mouse_right_click() && within_inventory) {
		int slot = get_itemslotn(inv, bbox, mx, my);
		if (slot != -1 && inv.slot_filled(slot)) {
			slot_selected = slot;
			return true;
		}
	}

	/* Drop a dragged item */
	if (slot_selected > -1 && gs->mouse_right_release()) {
		int slot = get_itemslotn(inv, bbox, mx, my);

		if (slot == -1 || slot == slot_selected) {
			queued_actions.push_back(
					game_action(gs, p, GameAction::DROP_ITEM, slot_selected));
		} else {
			queued_actions.push_back(
					game_action(gs, p, GameAction::REPOSITION_ITEM,
							slot_selected, 0, 0, slot));
		}
		return true;
	}

	return false;
}

