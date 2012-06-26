/**
 * GameHud:
 *  Various components of the overlay display & side bar
 *  Includes inventory manipulation
 */

#ifndef GAMEHUD_H_
#define GAMEHUD_H_

#include <deque>

#include "../../display/display.h"
#include "../../display/GLImage.h"

#include "../../util/game_basic_structs.h"

#include "../objects/GameInst.h"

#include "../GameAction.h"
#include "../GameInstSet.h"

#include "detail/ActionBar.h"

struct PlayerInst;


/* Component of GameState that draws statistic overview */
class GameHud {
public:
	/* Draw the game statistics overlay */
	void draw(GameState* gs);

	void step(GameState* gs);

	/*Returns whether has handled event completely or not*/
	bool handle_event(GameState* gs, SDL_Event *event);
	/* Handles clicks, etc */
	void queue_io_actions(GameState* gs, PlayerInst* player, std::deque<GameAction>& queued_actions);

	/*Location of the minimap on the screen*/
	BBox minimap_bbox(GameState* gs);

	GameHud(int x, int y, int width, int height, int view_width, int view_height);
	~GameHud();

	/* Width of overlay */
	int width() { return _width; }
	/* Height of overlay */
	int height() { return _height; }

	/* Reset the selected item after action is finished, otherwise drawing will lag a step
	 * Used in PlayerInstActions.cpp*/
	void reset_slot_selected(){
		item_slot_selected = -1;
	}
private:
	//TODO use HudView
	enum HudView {
		INVENTORY, STATS, SHOP
	};
	/* Helper method for drawing mini-map based on game information */
	void draw_minimap(GameState* gs, const BBox& bbox, float scale);

	ActionBar action_bar;
	ActionSlot dragging_action;

	BBox minimapbox;
	int x, y, _width, _height;
	Colour bg_colour;
	char* minimap_arr;
	GLimage minimap_buff;
	int item_slot_selected;

	std::deque<GameAction> queued_actions;
};



#endif /* GAMEHUD_H_ */
