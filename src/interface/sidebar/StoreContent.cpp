/*
 * StoreContent.cpp:
 *  Represents an interactive view of a store for the side bar
 */

#include "../../stats/item_data.h"
#include "../../display/sprite_data.h"

#include "../../display/display.h"

#include "../../display/colour_constants.h"
#include "../console_description_draw.h"

#include "../../gamestate/GameState.h"

#include "../../objects/player/PlayerInst.h"

#include "StoreContent.h"

void StoreContent::draw(GameState* gs) const {
}

bool StoreContent::handle_io(GameState* gs, ActionQueue & queued_actions) {
	return true;
}

