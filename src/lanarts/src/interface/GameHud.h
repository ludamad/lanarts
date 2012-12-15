/**
 * GameHud:
 *  Various components of the overlay display & side bar
 *  Includes inventory manipulation
 */

#ifndef GAMEHUD_H_
#define GAMEHUD_H_

#include <deque>

#include "../display/GLImage.h"
#include "../display/display.h"

#include "../gamestate/ActionQueue.h"
#include "../gamestate/GameAction.h"

#include "../objects/GameInst.h"

#include "../lanarts_defines.h"

#include "sidebar/Sidebar.h"

#include "sidebar/SidebarNavigator.h"
#include "ActionBar.h"
#include "GameTextConsole.h"

class PlayerInst;

/* Component of GameState that draws statistic overview */
class GameHud {
public:
	/* Draw the game statistics overlay */
	void draw(GameState* gs);

	void step(GameState* gs);

	/*Returns whether has handled event completely or not*/
	bool handle_event(GameState* gs, SDL_Event *event);
	/* Handles clicks, etc */
	bool handle_io(GameState* gs, ActionQueue& queued_actions);

	/*Location of the minimap on the screen*/
	BBox minimap_bbox(GameState* gs);

	GameHud(const BBox& sidebar_box, const BBox& view_box);

	/* Width of overlay */
	int width() {
		return sidebar_box.width();
	}
	/* Height of overlay */
	int height() {
		return sidebar_box.height();
	}
	GameChat& game_chat() {
		return console.game_chat();
	}
	GameTextConsole& game_console() {
		return console;
	}

	BBox sidebar_content_area() {
		return sidebar.sidebar_content_area();
	}

	void override_sidebar_contents(SidebarContent* overlay) {
		sidebar.override_sidebar_contents(overlay);
	}

	/* Reset the selected item after action is finished, otherwise drawing will lag a step
	 * Used in PlayerInstActions.cpp*/
	void reset_slot_selected();
private:
	GameTextConsole console;
	Sidebar sidebar;
	ActionBar action_bar;
	ActionSlot dragging_action;

	BBox sidebar_box, view_box;
	Colour bg_colour;

	ActionQueue queued_actions;
};

#endif /* GAMEHUD_H_ */
