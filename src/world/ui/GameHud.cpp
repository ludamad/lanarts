/*
 * GameHud.cpp
 *  Various components of the overlay display & side bar
 *  Includes inventory manipulation
 */

#include "../../display/display.h"

#include "../../data/tile_data.h"
#include "../../data/item_data.h"
#include "../../data/spell_data.h"
#include "../../data/sprite_data.h"
#include "../../data/weapon_data.h"

#include "../../gamestats/Inventory.h"

#include "../../util/math_util.h"

#include "../GameState.h"
#include "../GameInstSet.h"

#include "../objects/GameInst.h"
#include "../objects/PlayerInst.h"

#include "GameHud.h"

#include "detail/SpellsContent.h"

const int INVENTORY_POSITION = 342;

BBox GameHud::minimap_bbox(GameState* gs) {
	int minimap_relposx = 20, minimap_relposy = 64 + 45;
	int sx = sidebar_box.x1 + minimap_relposx, sy = sidebar_box.y1
			+ minimap_relposy;
	BBox bbox(sx, sy, sx + gs->get_level()->tile_width(),
			sy + gs->get_level()->tile_height());
	int minimap_w = bbox.width(), minimap_h = bbox.height();
	int minimap_x = bbox.x1 + (128 - minimap_w) / 2, minimap_y = bbox.y1
			+ (128 - minimap_w) / 2;
	return BBox(minimap_x, minimap_y, minimap_x + minimap_w,
			minimap_y + minimap_h);
}

void GameHud::step(GameState *gs) {
	console.step(gs);
	action_bar.step(gs);
	sidebar.step(gs);
}

bool GameHud::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* player = gs->local_player();
	bool mouse_within_view = gs->mouse_x() < gs->window_view().width;
	int level = gs->get_level()->roomid, frame = gs->frame();

	Inventory inv = player->inventory();

	for (int i = 0; i < this->queued_actions.size(); i++) {
		queued_actions.push_back(this->queued_actions[i]);
	}
	this->queued_actions.clear();

	if (action_bar.handle_io(gs, queued_actions)) {
		return true;
	}
	if (sidebar.handle_io(gs, queued_actions)) {
		return true;
	}
	return false;
}

bool GameHud::handle_event(GameState* gs, SDL_Event* event) {
	if (game_chat().handle_event(gs, event)) {
		return true;
	}
	return false;
}

void GameHud::draw(GameState* gs) {
	gl_set_drawing_area(0, 0, sidebar_box.x2, sidebar_box.y2);
	gl_draw_rectangle(sidebar_box.x1, sidebar_box.y1, width(), height(),
			bg_colour);

	PlayerInst* player_inst = (PlayerInst*)gs->get_instance(
			gs->local_playerid());
	if (!player_inst)
		return;

	action_bar.draw(gs);
	sidebar.draw(gs);
	// Must draw console after other components have chance to draw content there
	console.draw(gs);
}

static BBox action_bar_area(const BBox& view_box) {
	return BBox(view_box.x1, view_box.y2 - TILE_SIZE, view_box.x2, view_box.y2);
}
static BBox text_console_area(const BBox& view_box) {
	const int TEXT_CONSOLE_HEIGHT = 100;
	return BBox(view_box.x1, view_box.y1, view_box.x2,
			view_box.y1 + TEXT_CONSOLE_HEIGHT);
}

GameHud::GameHud(const BBox& sidebar_box, const BBox& view_box) :
		console(text_console_area(view_box)), sidebar(sidebar_box), action_bar(
				action_bar_area(view_box)), sidebar_box(sidebar_box), view_box(
				view_box), bg_colour(0, 0, 0), minimap_arr(NULL) {
}
GameHud::~GameHud() {
	delete[] minimap_arr;
}

void GameHud::reset_slot_selected() {
	sidebar.reset_slot_selected();
}

