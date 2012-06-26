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

static void draw_spells_known(const BBox& bbox, SpellsKnown& spells) {
	const int spell_n = spells.amount();
	int spellidx = 0;

	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
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

void SpellsContent::draw(GameState* gs) const {
	PlayerInst* p = (PlayerInst*) gs->get_instance(gs->local_playerid());

	if (!p) {
		return;
	}

	draw_spells_known(bbox, p->spells_known());
}

