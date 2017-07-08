/*
 * SpellsContent.cpp:
 *  Draws known spells in a grid, for the side bar
 */

#include <ldraw/draw.h>

#include "draw/colour_constants.h"

#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"

#include "objects/PlayerInst.h"

#include "stats/SpellsKnown.h"
#include "stats/SpellEntry.h"

#include "../console_description_draw.h"

#include "SpellsContent.h"

static void draw_spells_known(GameState* gs, const BBox& bbox,
		SpellsKnown& spells, int ind_low, int ind_high) {
	const int spell_n = spells.amount();
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int spellidx = ind_low;
	int selected_spell = gs->local_player()->spell_selected();

	LuaValue& handler = gs->local_player()->input_source().value;
	int slot_highlighted = lmethod_call<int>(handler, "slot_highlighted");

	ldraw::draw_rectangle_outline(COL_UNFILLED_OUTLINE, bbox);
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			bool dummy_slot = (spellidx >= spell_n);
			if (spellidx >= 40) {
				break;
			}
			auto draw_spell = [&]() {
				if (dummy_slot) {
					return;
				}
				spell_id spell = spells.get(spellidx);
				SpellEntry& spl_entry = res::spell(spell);
				draw_sprite(spl_entry.sprite, x, y);
                lmeth(handler, "draw_spell_ui_hint", Pos {x, y}, spellidx);
			};
			BBox entry_box(x, y, x + TILE_SIZE, y + TILE_SIZE);
			draw_spell();
			Colour bbox_col = COL_FILLED_OUTLINE;
			auto draw_topbar_overlay = [&]() {
				if (dummy_slot) {
					return;
				}
				if (entry_box.contains(mx, my)) {
					bbox_col = COL_GOLD;
					draw_console_spell_description(gs, res::spell(spells.get(spellidx)));
				} else if (slot_highlighted == spellidx) {
					draw_console_spell_description(gs, res::spell(spells.get(spellidx)));
				}
			};
			draw_topbar_overlay();
            if (slot_highlighted == spellidx) {
                bbox_col = COL_PALE_GREEN;
            } else if (dummy_slot) {
                bbox_col = COL_UNFILLED_OUTLINE;
            }
			ldraw::draw_rectangle_outline(bbox_col, entry_box);
			spellidx++;
		}
	}
}

const int SPELLS_PER_PAGE = 40;

void SpellsContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	int min_spell = SPELLS_PER_PAGE * page_number, max_spell = min_spell
			+ SPELLS_PER_PAGE;
	draw_spells_known(gs, bbox, p->spells_known(), min_spell, max_spell);
}

int SpellsContent::amount_of_pages(GameState* gs) {
	PlayerInst* p = gs->local_player();

	SpellsKnown& spells = p->spells_known();
	int spells_n = spells.amount();
	/* Add SPELLS_PER_PAGE - 1 so that 0 spells need 0 pages, 1 spell needs 1 page, etc*/
	int spell_pages = (spells_n + SPELLS_PER_PAGE - 1) / SPELLS_PER_PAGE;

	return spell_pages;
}

static bool handle_io_spells_known(GameState* gs, ActionQueue& queued_actions,
		const BBox& bbox, SpellsKnown& spells, int ind_low, int ind_high) {
	LuaValue& handler = gs->local_player()->input_source().value;
	lmeth(handler, "handle_spells");
	return false;
}

bool SpellsContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();

	int min_spell = SPELLS_PER_PAGE * page_number, max_spell = min_spell
			+ SPELLS_PER_PAGE;
	return handle_io_spells_known(gs, queued_actions, bbox, p->spells_known(),
			min_spell, max_spell);
}

