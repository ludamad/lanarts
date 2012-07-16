/*
 * EquipmentContent.cpp:
 *  Represents an interactive view of equipped items for the side bar
 */

#include "../../../data/item_data.h"
#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

#include "../../../util/colour_constants.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "EquipmentContent.h"

EquipmentContent::~EquipmentContent() {
}

void EquipmentContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	Equipment& inv = p->equipment();

//	draw_player_inventory(gs, inv, bbox, min_item, max_item, slot_selected);
}

int EquipmentContent::amount_of_pages(GameState* gs) {
	return 1;
}

bool EquipmentContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	return false;
}

