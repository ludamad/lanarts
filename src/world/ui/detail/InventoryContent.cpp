/*
 * InventoryContent.cpp:
 *  Represents an interactive view of the inventory for the side bar
 */

#include "../../../data/item_data.h"
#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

#include "../../../data/sprite_data.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "InventoryContent.h"

static void draw_player_inventory_slot(GameState* gs, ItemSlot& itemslot, int x,
		int y) {
	if (itemslot.amount > 0) {
		ItemEntry& itemd = itemslot.item.item_entry();
		GLimage& itemimg = game_sprite_data[itemd.sprite_number].img();
		gl_draw_image(itemimg, x, y);
		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + 1, y + 1, "%d",
				itemslot.amount);
	}
}
static void draw_player_inventory(GameState* gs, Inventory& inv,
		const BBox& bbox, int min_slot, int max_slot, int slot_selected = -1) {
	int slot = min_slot;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (slot >= max_slot)
				break;

			ItemSlot& itemslot = inv.get(slot);

			Colour outline(43, 43, 43);
			if (itemslot.amount > 0 && slot != slot_selected)
				outline = Colour(120, 115, 110);

			gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE, outline);
			if (slot != slot_selected)
				draw_player_inventory_slot(gs, itemslot, x, y);

			slot++;
		}
	}

	if (slot_selected != -1) {
		draw_player_inventory_slot(gs, inv.get(slot_selected),
				gs->mouse_x() - TILE_SIZE / 2, gs->mouse_y() - TILE_SIZE / 2);
	}
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

int InventoryContent::get_itemslotn(GameState* gs, int mx, int my) {
	int posx = (mx - bbox.x1) / TILE_SIZE;
	int posy = (my - bbox.y1) / TILE_SIZE;
	int slot = 5 * posy + posx;
	if (slot < 0 || slot >= INVENTORY_SIZE)
		return -1;
	return slot;
}

bool InventoryContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();
	int level = gs->get_level()->roomid, frame = gs->frame();

	if (gs->mouse_left_click() && bbox.contains(gs->mouse_x(), gs->mouse_y())) {
		Inventory inv = p->inventory();

		int slot = get_itemslotn(gs, gs->mouse_x(), gs->mouse_y());
		if (slot >= 0 && slot < INVENTORY_SIZE && inv.get(slot).amount > 0) {
			queued_actions.push_back(
					GameAction(p->id, GameAction::USE_ITEM, frame, level, slot,
							p->x, p->y));
			return true;
		}
	}

	return false;
}

