/*
 * ConfigContent.cpp:
 *  Visual toggle of configuration options in side bar
 */

#include "ConfigContent.h"

#include "../../display/sprite_data.h"

#include "../../display/display.h"

#include "../../display/colour_constants.h"

#include "../console_description_draw.h"

#include "../../gamestate/GameState.h"

static void draw_option(GameState* gs, const BBox& bbox, bool option_set,
		sprite_id sprite, const char* text) {
	option_set = false;
	Colour bbox_col = COL_FILLED_OUTLINE;
	if (bbox.contains(gs->mouse_pos())) {
		bbox_col = COL_GOLD;
	}

	gl_draw_sprite(sprite, bbox.x1, bbox.y1,
			COL_WHITE.with_alpha(option_set ? 255 : 122));
	/* Draw item name */
	gl_printf_bounded(gs->primary_font(), option_set ? COL_WHITE : COL_GRAY,
			bbox.x1 + TILE_SIZE * 1.25, bbox.y1 + TILE_SIZE / 2,
			bbox.width() - TILE_SIZE, true, "%s", text);
	gl_draw_rectangle_outline(bbox, bbox_col);
}

void ConfigContent::draw(GameState* gs) const {
	GameSettings& settings = gs->game_settings();
	gl_draw_rectangle_outline(bbox, COL_UNFILLED_OUTLINE);

	BBox entry_box(bbox.x1, bbox.y1, bbox.x2, bbox.y1 + TILE_SIZE);
	draw_option(gs, entry_box, settings.autouse_mana_potions,
			get_sprite_by_name("mana_potion"), "Auto-Use Mana Potions");
	entry_box = entry_box.translated(0, TILE_SIZE);
	draw_option(gs, entry_box, settings.autouse_mana_potions,
			get_sprite_by_name("health_potion"), "Auto-Use Health Potions");
}

bool ConfigContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	GameSettings& settings = gs->game_settings();
	return false;
}

