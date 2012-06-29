/*
 * SidebarNavigator.h:
 *  Handles navigation of content in the side bar
 */

#ifndef SIDEBARNAVIGATOR_H_
#define SIDEBARNAVIGATOR_H_

#include <vector>

#include "../../../util/ActionQueue.h"

#include "../../GameAction.h"

class SidebarContent;

struct NavigationSprites {
	sprite_id left_arrow, right_arrow;
	sprite_id inventory_icon, stats_icon, spells_icon;
	NavigationSprites();
};

class SidebarNavigator {
public:
	SidebarNavigator(const BBox& side_bar, const BBox& main_content);
	~SidebarNavigator();

	void draw(GameState* gs);
	void step(GameState* gs);

	bool handle_io(GameState* gs, ActionQueue& queued_actions);

	SidebarContent* current_content();
private:
	NavigationSprites sprites;
	BBox side_bar, main_content;

	enum view_t {
		INVENTORY, SPELLS, STATS, SHOP
	};
	view_t viewtype;
	SidebarContent* inventory, *spells, *shop;
};

#endif /* SIDEBARNAVIGATOR_H_ */
