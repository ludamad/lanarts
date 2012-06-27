/*
 * InventoryContent.cpp:
 *  Represents an interactive view of the inventory for the side bar
 */

#include "../../../data/item_data.h"
#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

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
static void draw_player_inventory(GameState* gs, Inventory& inv, const BBox& bbox, int min_slot, int max_slot, int slot_selected = -1) {
	int slot = min_slot;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (slot >= max_slot)
				break;

			ItemSlot& itemslot = inv.get(slot);

			Colour outline(43, 43, 43);
			if (itemslot.amount > 0 && slot != slot_selected)
				outline = Colour(120, 115, 110);

			gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE,
					outline);
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
	PlayerInst* p = (PlayerInst*) gs->get_instance(gs->local_playerid());

	if (!p) {
		return;
	}

	Inventory& inv = p->inventory();
	int min_item = ITEMS_PER_PAGE * page_number, max_item = min_item
			+ ITEMS_PER_PAGE;
	draw_player_inventory(gs, inv, bbox, min_item, max_item);
}

int InventoryContent::amount_of_pages(GameState* gs) {
	PlayerInst* p = (PlayerInst*) gs->get_instance(gs->local_playerid());

	if (!p) {
		return 0;
	}

	SpellsKnown& spells = p->spells_known();
	int items_n = spells.amount();
	/* Add SPELLS_PER_PAGE - 1 so that 0 spells need 0 pages, 1 spell needs 1 page, etc*/
	int item_pages = (items_n + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;

	return item_pages;
}
