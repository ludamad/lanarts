/*
 * ActionBar.cpp:
 *  Holds the contents of the action bar.
 */

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
bool ActionBar::handle_click(GameState *gs) {
	return false;
}

int ActionBar::get_selected_slot(int mx, int my) {
	int action_start_x = bbox.x1 + EQUIP_SLOT_WIDTH;

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
