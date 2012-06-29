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

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

static void draw_spell_icon(spell_id spell, int x, int y) {
	SpellEntry& spl_entry = game_spell_data.at(spell);
	SpriteEntry& spr_entry = game_sprite_data.at(spl_entry.sprite);
	gl_draw_image(spr_entry.img(), x, y);
}

static void draw_spells_known(const BBox& bbox, SpellsKnown& spells,
		int ind_low, int ind_high) {
	const int spell_n = spells.amount();
	int spellidx = ind_low;

	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (spellidx >= ind_high)
				break;

			bool filledslot = spellidx < spell_n;
			Colour outline = filledslot ? FILLED_OUTLINE : UNFILLED_OUTLINE;

			gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE, outline);

			if (filledslot) {
				draw_spell_icon(spells.get(spellidx), x, y);
			}

			spellidx++;
		}
	}
}

const int SPELLS_PER_PAGE = 40;

void SpellsContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	int min_spell = SPELLS_PER_PAGE * page_number, max_spell = min_spell
			+ SPELLS_PER_PAGE;
	draw_spells_known(bbox, p->spells_known(), min_spell, max_spell);
}

int SpellsContent::amount_of_pages(GameState* gs) {
	PlayerInst* p = gs->local_player();

	SpellsKnown& spells = p->spells_known();
	int spells_n = spells.amount();
	/* Add SPELLS_PER_PAGE - 1 so that 0 spells need 0 pages, 1 spell needs 1 page, etc*/
	int spell_pages = (spells_n + SPELLS_PER_PAGE - 1) / SPELLS_PER_PAGE;

	return spell_pages;
}

