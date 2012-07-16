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
	void reset_slot_selected();

private:
	enum view_t {
		INVENTORY, SPELLS, EQUIPMENT, STATS, ENEMIES, CONFIG
	};
	struct NavigationOption {
		NavigationOption(sprite_id icon, SidebarContent* content,
				const BBox& icon_bbox);
		~NavigationOption();

		void draw_icon(GameState* gs, bool selected = false);

		sprite_id icon;
		SidebarContent* content;
		BBox icon_bbox;
	};
	NavigationOption& current_option();

	bool handle_icon_io(GameState* gs, NavigationOption& option, view_t value);

	NavigationSprites sprites;
	BBox side_bar, main_content;
	view_t view;
	NavigationOption inventory, equipment, spells, enemies, config;
};

#endif /* SIDEBARNAVIGATOR_H_ */
