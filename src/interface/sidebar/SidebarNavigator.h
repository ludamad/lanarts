/*
 * SidebarNavigator.h:
 *  Handles navigation of content in the side bar
 */

#ifndef SIDEBARNAVIGATOR_H_
#define SIDEBARNAVIGATOR_H_

#include <vector>

#include "gamestate/ActionQueue.h"

#include "gamestate/GameAction.h"
#include <memory>

class SidebarContent;

class SidebarNavigator {
public:
	SidebarNavigator(const BBox& side_bar, const BBox& main_content);
	~SidebarNavigator();

	void draw(GameState* gs);
	void step(GameState* gs);

	bool handle_io(GameState* gs, ActionQueue& queued_actions);

	SidebarContent* current_content();
	void reset_slot_selected();

	void override_sidebar_contents(SidebarContent* overlay) {
		content_overlay = overlay;
	}

	BBox sidebar_content_area() {
		return main_content;
	}

private:
	enum view_t {
		INVENTORY, SPELLS, EQUIPMENT, STATS, ENEMIES, CONFIG
	};
	struct NavigationOption {
		NavigationOption(const std::string& icon, std::shared_ptr<SidebarContent> content,
				const BBox& icon_bbox);
		~NavigationOption();

		void draw_icon(GameState* gs, bool selected = false);

		std::string iconsprite;
		std::shared_ptr<SidebarContent> content;
		BBox icon_bbox;
	};
	NavigationOption& current_option();

	bool handle_icon_io(GameState* gs, NavigationOption& option, view_t value);

	BBox side_bar, main_content;
	view_t view;
	SidebarContent* content_overlay;
	NavigationOption inventory, equipment, spells, enemies, config;
};

#endif /* SIDEBARNAVIGATOR_H_ */
