/*
 * EquipmentContent.cpp:
 *  Represents an interactive view of equipped items for the side bar
 */

#include "../../stats/item_data.h"
#include "../../stats/weapon_data.h"

#include "../../display/sprite_data.h"

#include "../../display/display.h"

#include "../../display/colour_constants.h"

#include "../console_description_draw.h"

#include "../../gamestate/GameState.h"

#include "../../objects/player/PlayerInst.h"

#include "EquipmentContent.h"

EquipmentContent::~EquipmentContent() {
}

static void draw_weapon(GameState* gs, Equipment& eqp, BBox bbox){
	Colour bbox_col = COL_FILLED_OUTLINE;
	draw_item_icon_and_name(gs, eqp.weapon.item_entry(), COL_WHITE, bbox.x1 , bbox.y1);
	gl_draw_rectangle_outline(bbox, bbox_col);
}

static void draw_armour(GameState* gs, Equipment& eqp, BBox bbox){
	Colour bbox_col = COL_UNFILLED_OUTLINE;
	if(eqp.has_armour()){
		bbox_col = COL_FILLED_OUTLINE;
	}
	draw_item_icon_and_name(gs, eqp.armour.item_entry(), COL_WHITE, bbox.x1, bbox.y1);
}

void EquipmentContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	Equipment& eqp = p->equipment();
	
	gl_draw_rectangle_outline(bbox, COL_UNFILLED_OUTLINE);
	
	int x = bbox.x1, ex = bbox.x2, y = bbox.y1;
	
	BBox entry_box(x, y, ex, y + TILE_SIZE);
	
	Colour bbox_col = COL_UNFILLED_OUTLINE;
	
	draw_weapon(gs, eqp, entry_box);

//	draw_player_inventory(gs, inv, bbox, min_item, max_item, slot_selected);
}

int EquipmentContent::amount_of_pages(GameState* gs) {
	return 1;
}

bool EquipmentContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	return false;
}

