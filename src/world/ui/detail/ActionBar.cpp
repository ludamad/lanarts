/*
 * ActionBar.cpp:
 *  Holds the contents of the action bar.
 */

#include "../../../util/game_basic_structs.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "ActionBar.h"

ActionBar::ActionBar(int x, int y, int w, int h, int max_actions) :
		x(x), y(y), w(w), h(h), actions(max_actions) {
}

const int EQUIP_SLOT_WIDTH = TILE_SIZE * 2;
const int ACTION_SLOT_WIDTH = TILE_SIZE;
const int SLOT_HEIGHT = TILE_SIZE;

bool ActionBar::is_within_equipped(int mx, int my) {
	BBox bbox(x, y, x + EQUIP_SLOT_WIDTH, y + SLOT_HEIGHT);
	return bbox.contains(mx, my);
}

bool ActionBar::is_within_actionbar(int mx, int my) {
	int action_start_x = x + EQUIP_SLOT_WIDTH;
	BBox bbox(action_start_x, y, x + w, y + h);
	return bbox.contains(mx, my);
}

/* Return which slot the mouse is hovering over */
int ActionBar::get_selected_slot(int mx, int my) {
	int action_start_x = x + EQUIP_SLOT_WIDTH;

	if (!is_within_actionbar(mx, my)) {
		return -1;
	}

	return (mx - action_start_x) / ACTION_SLOT_WIDTH;
}

//static void draw_player_weapon_actionbar(GameState* gs, PlayerInst* player,
//		int x, int y) {
//	Colour outline = outline_col(player->spell_selected() == -1);
//	Equipment& equipment = player->equipment();
//
//	if (equipment.projectile == -1) {
//		gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE, outline);
//	} else {
//		gl_draw_rectangle_outline(x, y, TILE_SIZE * 2, TILE_SIZE, outline);
//		ProjectileEntry& ptype = equipment.projectile.projectile_entry();
//		gl_draw_image(game_sprite_data[ptype.item_sprite].img(), x + TILE_SIZE,
//				y);
//		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + TILE_SIZE + 1,
//				y + 1, "%d", player->equipment().projectile_amnt);
//	}
//
//	WeaponEntry& wentry = player->weapon_type().weapon_entry();
//	gl_draw_image(game_sprite_data[wentry.item_sprite].img(), x, y);
//}

// Frame event handling
void ActionBar::draw(GameState *gs) const {
	PlayerInst* player = (PlayerInst*) gs->get_instance(gs->local_playerid());

	if (!player) {
		return;
	}

	Equipment& equipment = player->equipment();
	equipment.weapon;
}

void ActionBar::step(GameState *gs) {
	// Nothing to be done, placeholder
}
