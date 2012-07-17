/*
 * ActionBar.cpp:
 *  Handles drawing and state of the action bar.
 */

#include "../../../data/item_data.h"
#include "../../../data/spell_data.h"
#include "../../../data/projectile_data.h"
#include "../../../data/weapon_data.h"

#include "../../../util/colour_constants.h"
#include "../../../util/content_draw_util.h"
#include "../../../util/game_basic_structs.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "ActionBar.h"

ActionBar::ActionBar(const BBox& bbox, int max_actions) :
		bbox(bbox), actions(max_actions) {
}

const int EQUIP_SLOT_WIDTH = TILE_SIZE * 2;

const int SLOT_WIDTH = TILE_SIZE;
const int SLOT_HEIGHT = TILE_SIZE;

static bool is_within_equipped_weapon(const BBox& bbox, int mx, int my) {
	BBox equip_box(bbox.x1, bbox.y1, bbox.x1 + SLOT_WIDTH,
			bbox.y1 + SLOT_HEIGHT);
	return equip_box.contains(mx, my);
}
static bool is_within_equipped_projectile(const BBox& bbox, int mx, int my) {
	BBox equip_box(bbox.x1 + SLOT_WIDTH, bbox.y1, bbox.x1 + EQUIP_SLOT_WIDTH,
			bbox.y1 + SLOT_HEIGHT);
	return equip_box.contains(mx, my);
}

static bool is_within_actionbar(const BBox& bbox, int mx, int my) {
	BBox actionbar_box(bbox);
	actionbar_box.x1 += EQUIP_SLOT_WIDTH;
	return actionbar_box.contains(mx, my);
}

/* Return which slot the mouse is hovering over */
static int get_selected_slot(const BBox& bbox, int mx, int my) {
	int action_start_x = bbox.x1 + EQUIP_SLOT_WIDTH;

	if (!is_within_actionbar(bbox, mx, my)) {
		return -1;
	}

	return (mx - action_start_x) / SLOT_WIDTH;
}

static bool handle_equip_slot_io(GameState* gs, PlayerInst* p, const BBox& bbox,
		ActionQueue& queued_actions) {
	int mx = gs->mouse_x(), my = gs->mouse_y();
	bool leftdown = gs->mouse_left_down(), rightdown = gs->mouse_right_down();

	/* Check whether to de-equip weapon */
	if (rightdown && is_within_equipped_weapon(bbox, mx, my)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::DEEQUIP_ITEM,
						ItemEntry::WEAPON));
		return true;
	}

	/* Check whether to de-equip projectile */
	if (rightdown && is_within_equipped_projectile(bbox, mx, my)) {
		if (p->projectile().valid_projectile()) {
			queued_actions.push_back(
					game_action(gs, p, GameAction::DEEQUIP_ITEM,
							ItemEntry::PROJECTILE));
			return true;
		}
	}

	/* Check whether to select weapon as active action */
	bool selects_projectile = p->projectile().valid_projectile()
			&& is_within_equipped_projectile(bbox, mx, my);
	if (leftdown
			&& (is_within_equipped_weapon(bbox, mx, my) || selects_projectile)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::CHOSE_SPELL, -1));
		return true;
	}
	return false;
}

static bool handle_spell_slot_io(GameState* gs, PlayerInst* p, const BBox& bbox,
		ActionQueue& queued_actions) {
	int mx = gs->mouse_x(), my = gs->mouse_y();
	bool leftdown = gs->mouse_left_down(), rightdown = gs->mouse_right_down();

	/* Check if a spell is selected */
	int spell_slot = get_selected_slot(bbox, mx, my);

	if (leftdown && spell_slot > -1) {
		if (spell_slot < p->spells_known().amount()) {
			queued_actions.push_back(
					game_action(gs, p, GameAction::CHOSE_SPELL, spell_slot));
		}
		return true; // Ensures mouse actions are filtered when clicking on action bar
	}

	if (rightdown && spell_slot > -1) {
		if (spell_slot < p->spells_known().amount()) {
			queued_actions.push_back(
					game_action(gs, p, GameAction::CHOSE_SPELL, -1));
		}
		return true; // Ensures mouse actions are filtered when clicking on action bar
	}
	return false;
}

/* Return which slot the mouse is hovering over
 * NOTE: This function should return true when the left mouse button is down
 * and within the action bar, so that other click events (eg using an attack)
 * do not process */
bool ActionBar::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();

	if (handle_equip_slot_io(gs, p, bbox, queued_actions)) {
		return true;
	}

	if (handle_spell_slot_io(gs, p, bbox, queued_actions)) {
		return true;
	}

	return false;
}

static void draw_player_weapon_actionbar(GameState* gs, PlayerInst* player,
		int x, int y) {
	int mx = gs->mouse_x(), my = gs->mouse_y();
	Weapon& weapon = player->weapon();
	bool weapon_selected = player->spell_selected() == -1;
	Colour outline =
			weapon_selected ? COL_SELECTED_OUTLINE : COL_FILLED_OUTLINE;

	/* Draw only enough space for weapon if no projectile used */
	bool draw_with_projectile = player->projectile().valid_projectile();

	BBox weaponbox(x + 1, y, x + 1 + TILE_SIZE, y + TILE_SIZE);
	BBox equipbox(weaponbox);
	if (draw_with_projectile) {
		equipbox.x2 += TILE_SIZE;
	}

	if (equipbox.contains(mx, my) && !weapon_selected) {
		outline = COL_PALE_YELLOW;
	}

	if (weaponbox.contains(mx, my)) {
		draw_console_item_description(gs, weapon.as_item());
	}

	/* Draw weapon*/
	WeaponEntry& wentry = weapon.weapon_entry();
	gl_draw_image(game_sprite_data[wentry.item_sprite].img(), x, y);

	if (draw_with_projectile) {
		BBox projectilebox(weaponbox.translated(TILE_SIZE, 0));

		Projectile& projectile = player->projectile();

		if (projectilebox.contains(mx, my)) {
			draw_console_item_description(gs, projectile.as_item());
		}

		ProjectileEntry& ptype = projectile.projectile_entry();
		gl_draw_image(game_sprite_data[ptype.item_sprite].img(), x + TILE_SIZE,
				y);
		/* Draw projectile amount */
		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + TILE_SIZE + 1,
				y + 1, "%d", player->equipment().projectile_amnt);
	}

	gl_draw_rectangle_outline(equipbox, outline);
}

static void draw_player_spell_actionbar(GameState* gs, PlayerInst* player,
		const BBox& bounds) {

	int mx = gs->mouse_x(), my = gs->mouse_y();
	SpellsKnown& spells = player->spells_known();

	const int spell_n = spells.amount();
	const int sx = bounds.x1 + 1, sy = bounds.y1;

	for (int i = 0; i < spell_n; i++) {
		spell_id spell = spells.get(i);
		SpellEntry& spl_entry = game_spell_data.at(spell);
		SpriteEntry& spr_entry = game_sprite_data.at(spl_entry.sprite);
		gl_draw_image(spr_entry.img(), sx + i * TILE_SIZE, sy);
	}

	for (int x = sx, spellidx = 0; x < bounds.x2; x += TILE_SIZE, spellidx++) {
		BBox spellbox(x, sy, x + TILE_SIZE, sy + TILE_SIZE);
		bool is_selected = spellidx == player->spell_selected();
		Colour outline = COL_UNFILLED_OUTLINE;

		if (spellidx < spell_n) {
			spell_id spell = spells.get(spellidx);
			SpellEntry& spl_entry = game_spell_data.at(spell);

			outline = is_selected ? COL_SELECTED_OUTLINE : COL_FILLED_OUTLINE;

			if (spellbox.contains(mx, my)) {
				draw_console_spell_description(gs, spl_entry);
				if (!is_selected) {
					outline = COL_PALE_YELLOW;
				}
			}
		}

		gl_draw_rectangle_outline(spellbox, outline);

//		if (spellidx <= 9) {
//			gl_printf(gs->primary_font(), Colour(100, 255, 255),
//					x + TILE_SIZE - 12, sy + TILE_SIZE - 12, "%d", spellidx);
//		}
	}
}

// Frame event handling
void ActionBar::draw(GameState* gs) const {
	PlayerInst* player = gs->local_player();

	draw_player_weapon_actionbar(gs, player, bbox.x1, bbox.y1);
	draw_player_spell_actionbar(gs, player,
			BBox(bbox.x1 + EQUIP_SLOT_WIDTH, bbox.y1, bbox.x2, bbox.y2));
}

void ActionBar::step(GameState* gs) {
	// Nothing to be done, placeholder
}
