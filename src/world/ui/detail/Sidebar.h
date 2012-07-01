/*
 * Sidebar.h:
 *  Handles drawing & state of side bar
 */

#ifndef SIDEBAR_H_
#define SIDEBAR_H_

#include "../../../util/ActionQueue.h"
#include "../../../util/game_basic_structs.h"

#include "../../GameAction.h"

#include "Minimap.h"
#include "SidebarNavigator.h"

class GameState;

class Sidebar {
public:
	Sidebar(const BBox& sidebar_box);

	void draw(GameState* gs);
	void step(GameState* gs);
	bool handle_io(GameState* gs, ActionQueue& queued_actions);
	void reset_slot_selected() {
		navigator.reset_slot_selected();
	}

private:
	BBox sidebar_bounds;
	Minimap minimap;
	SidebarNavigator navigator;
};

#endif /* SIDEBAR_H_ */
