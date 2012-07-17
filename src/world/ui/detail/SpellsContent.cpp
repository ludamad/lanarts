/*
 * SpellsContent.cpp:
 *  Draws known spells in a grid, for the side bar
 */

#include "SpellsContent.h"

#include "../../../data/spell_data.h"
#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

#include "../../../gamestats/SpellsKnown.h"

#include "../../../util/colour_constants.h"
#include "../../../util/content_draw_util.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

static void draw_spells_known(GameState* gs, const BBox& bbox,
		SpellsKnown& spells, int ind_low, int ind_high) {
	const int spell_n = spells.amount();
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int spellidx = ind_low;
	int selected_spell = gs->local_player()->spell_selected();

	gl_draw_rectangle_outline(bbox, COL_UNFILLED_OUTLINE);

	int x = bbox.x1, ex = bbox.x2;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		if (spellidx >= spell_n)
			break;

		spell_id spell = spells.get(spellidx);
		SpellEntry& spl_entry = game_spell_data.at(spell);
		draw_spell_icon_and_name(gs, spl_entry, Colour(), x, y);

		BBox entry_box(x, y, ex - 2, y + TILE_SIZE);
		if (entry_box.contains(mx, my)) {
			if (spellidx != selected_spell) {
				gl_draw_rectangle_outline(entry_box, COL_FILLED_OUTLINE);
			}
			draw_console_spell_description(gs, spl_entry);
		}
		if (spellidx == selected_spell) {
			gl_draw_rectangle_outline(entry_box, COL_WHITE);
		}
		spellidx++;
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
	const int spell_n = spells.amount();
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int spellidx = ind_low;

	int x = bbox.x1, ex = bbox.x2;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		if (spellidx >= spell_n)
			break;

		BBox entry_box(x, y, ex - 2, y + TILE_SIZE);
		if (entry_box.contains(mx, my) && gs->mouse_left_click()) {
			queued_actions.push_back(
					game_action(gs, gs->local_player(), GameAction::CHOSE_SPELL,
							spellidx));
			return true;
		}
		spellidx++;
	}
	return false;
}

bool SpellsContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();

	int min_spell = SPELLS_PER_PAGE * page_number, max_spell = min_spell
			+ SPELLS_PER_PAGE;
	return handle_io_spells_known(gs, queued_actions, bbox, p->spells_known(),
			min_spell, max_spell);
}

