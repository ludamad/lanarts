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
	action_bar.step(gs);
	navigation.step(gs);
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
	int level = gs->get_level()->roomid, frame = gs->frame();

	bool mouse_within_view = gs->mouse_x() < gs->window_view().width;
	PlayerInst* player = gs->local_player();
	if (!player)
		return false;

	Inventory inv = player->inventory();
	const int SPELL_MAX = player->class_stats().xplevel >= 3 ? 2 : 1;

	bool mleft = event->button.button == SDL_BUTTON_LEFT;
	bool mright = event->button.button == SDL_BUTTON_RIGHT;

	//TODO: clean up & delegate to GameChat
	return false;
}

void GameHud::draw(GameState* gs) {
	gl_set_drawing_area(sidebar_box.x1, sidebar_box.y1, width(), height());
	gl_draw_rectangle(0, 0, width(), height(), bg_colour);

	PlayerInst* player_inst = (PlayerInst*) gs->get_instance(
			gs->local_playerid());
	if (!player_inst)
		return;

	gl_set_drawing_area(0, 0, sidebar_box.x2, sidebar_box.y2);

	int inv_w = width(), inv_h = height() - sidebar_box.y1 - TILE_SIZE;
	navigation.draw(gs);
	sidebar.draw(gs);
	action_bar.draw(gs);
}

static BBox content_area_box(const BBox& sidebar_box) {
	const int CONTENT_AREA_Y = 342;
	const int CONTENT_ROWS = 8;
	int sx = sidebar_box.x1, sy = sidebar_box.y1 + CONTENT_AREA_Y;
	int ex = sidebar_box.x2, ey = sy + CONTENT_ROWS * TILE_SIZE;
	return BBox(sx, sy, ex, ey);
}

static BBox action_bar_area(const BBox& view_box) {
	return BBox(view_box.x1, view_box.y2 - TILE_SIZE, view_box.x2, view_box.y2);
}

GameHud::GameHud(const BBox& sidebar_box, const BBox& view_box) :
		sidebar(sidebar_box), navigation(sidebar_box,
				content_area_box(sidebar_box)), action_bar(
				action_bar_area(view_box)), sidebar_box(sidebar_box), view_box(
				view_box), bg_colour(0, 0, 0), minimap_arr(NULL) {
	item_slot_selected = -1;
}
GameHud::~GameHud() {
	delete[] minimap_arr;
}
