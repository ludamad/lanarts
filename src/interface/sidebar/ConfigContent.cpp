/*
 * ConfigContent.cpp:
 *  Visual toggle of configuration options in side bar
 */

#include <ldraw/draw.h>

#include "draw/colour_constants.h"

#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"

#include "../components_draw.h"

#include "ConfigContent.h"

void ConfigContent::draw(GameState* gs) const {
	GameSettings& settings = gs->game_settings();
	ldraw::draw_rectangle_outline(COL_UNFILLED_OUTLINE, bbox);

	BBox entry_box(bbox.x1, bbox.y1, bbox.x2, bbox.y1 + TILE_SIZE);
	draw_option_box(gs, entry_box, settings.autouse_mana_potions,
			res::sprite_id("mana_potion"), "Auto-Use Mana Potions");
	entry_box = entry_box.translated(0, TILE_SIZE);
	draw_option_box(gs, entry_box, settings.autouse_health_potions,
			res::sprite_id("health_potion"), "Auto-Use Health Potions");
	entry_box = entry_box.translated(0, TILE_SIZE);
#ifndef __EMSCRIPTEN__
	draw_speed_box(gs, entry_box);
#endif
}

bool ConfigContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	GameSettings& settings = gs->game_settings();
	bool leftclick = gs->mouse_left_click();
	Pos mpos = gs->mouse_pos();

	BBox entry_box(bbox.x1, bbox.y1, bbox.x2, bbox.y1 + TILE_SIZE);
	if (leftclick && entry_box.contains(mpos)) {
		settings.autouse_mana_potions = !settings.autouse_mana_potions;
	}
	entry_box = entry_box.translated(0, TILE_SIZE);
	if (leftclick && entry_box.contains(mpos)) {
		settings.autouse_health_potions = !settings.autouse_health_potions;
	}
	entry_box = entry_box.translated(0, TILE_SIZE);
	if (leftclick && entry_box.contains(mpos)) {
		settings.time_per_step -= 3;
		if (settings.time_per_step < 10) {
			settings.time_per_step = 19;
		}
	}

	return false;
}

