/*
 * ActionBar.cpp:
 *  Holds the contents of the action bar.
 */

#include "../../../data/spell_data.h"
#include "../../../data/weapon_data.h"

#include "../../../util/colour_constants.h"
#include "../../../util/game_basic_structs.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "ActionBar.h"

ActionBar::ActionBar(const BBox& bbox, int max_actions) :
		bbox(bbox), actions(max_actions) {
}

const int EQUIP_SLOT_WIDTH = TILE_SIZE * 2;
const int ACTION_SLOT_WIDTH = TILE_SIZE;
const int SLOT_HEIGHT = TILE_SIZE;

bool ActionBar::is_within_equipped(int mx, int my) {
	BBox equip_box(bbox.x1, bbox.y1, bbox.x1 + EQUIP_SLOT_WIDTH,
			bbox.y1 + SLOT_HEIGHT);
	return equip_box.contains(mx, my);
}

bool ActionBar::is_within_actionbar(int mx, int my) {
	BBox actionbar_box(bbox);
	actionbar_box.x1 += EQUIP_SLOT_WIDTH;
	return actionbar_box.contains(mx, my);
}

/* Return which slot the mouse is hovering over */
bool ActionBar::handle_io(GameState* gs, ActionQueue& queued_actions) {
	return true;
}

int ActionBar::get_selected_slot(int mx, int my) {
	int action_start_x = bbox.x1 + EQUIP_SLOT_WIDTH;

	if (!is_within_actionbar(mx, my)) {
		return -1;
	}

	return (mx - action_start_x) / ACTION_SLOT_WIDTH;
}

static void draw_player_weapon_actionbar(GameState* gs, PlayerInst* player,
		int x, int y) {
	bool weapon_selected = player->spell_selected() == -1;
	Colour outline = weapon_selected ? SELECTED_OUTLINE : FILLED_OUTLINE;
	Equipment& equipment = player->equipment();

	if (equipment.projectile == -1) {
		gl_draw_rectangle_outline(x + 1, y, TILE_SIZE, TILE_SIZE, outline);
	} else {
		gl_draw_rectangle_outline(x + 1, y, TILE_SIZE * 2, TILE_SIZE, outline);
		ProjectileEntry& ptype = equipment.projectile.projectile_entry();
		gl_draw_image(game_sprite_data[ptype.item_sprite].img(), x + TILE_SIZE,
				y);
		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + TILE_SIZE + 1,
				y + 1, "%d", player->equipment().projectile_amnt);
	}

	WeaponEntry& wentry = player->weapon_type().weapon_entry();
	gl_draw_image(game_sprite_data[wentry.item_sprite].img(), x, y);
}

static void draw_player_spell_actionbar(GameState* gs, PlayerInst* player,
		const BBox& bounds) {

	SpellsKnown& spells = player->spells_known();
	const int spell_n = spells.amount();
	int spellidx = 0;

	int sx = bounds.x1, sy = bounds.y1;
	for (int x = sx; x < bounds.x2; x += TILE_SIZE) {
		bool is_selected = spellidx++ == player->spell_selected();
		Colour outline = is_selected ? SELECTED_OUTLINE : UNFILLED_OUTLINE;
		if (!is_selected && spellidx <= spell_n)
			outline = FILLED_OUTLINE;
		gl_draw_rectangle_outline(x, sy, TILE_SIZE, TILE_SIZE, outline);

		if (spellidx <= 9) {
			gl_printf(gs->primary_font(), Colour(100, 255, 255),
					x + TILE_SIZE - 12, sy + TILE_SIZE - 12, "%d", spellidx);
		}
	}
	for (int i = 0; i < spell_n; i++) {
		spell_id spell = spells.get(i);
		SpellEntry& spl_entry = game_spell_data.at(spell);
		SpriteEntry& spr_entry = game_sprite_data.at(spl_entry.sprite);
		gl_draw_image(spr_entry.img(), sx + i * TILE_SIZE, sy);
	}
}

// Frame event handling
void ActionBar::draw(GameState *gs) const {
	PlayerInst* player = gs->local_player();

	draw_player_weapon_actionbar(gs, player, bbox.x1, bbox.y1);
	draw_player_spell_actionbar(gs, player,
			BBox(bbox.x1 + EQUIP_SLOT_WIDTH, bbox.y1, bbox.x2, bbox.y2));
}

void ActionBar::step(GameState *gs) {
	// Nothing to be done, placeholder
}
