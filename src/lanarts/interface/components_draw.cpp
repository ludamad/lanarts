/*
 * components_draw.cpp:
 *  Helper methods for common interface components
 */
#include <draw/draw.h>
#include <draw/Font.h>
#include <draw/DrawOptions.h>

#include "../display/display.h"
#include "../draw/colour_constants.h"
#include "../display/sprite_data.h"

#include "../gamestate/GameState.h"

#include "components_draw.h"

void draw_setting_box(GameState* gs, const BBox& bbox, const Colour& bbox_col,
		sprite_id sprite, const Colour& sprite_col, const char* text,
		const Colour& text_col) {
	int y_offset = (bbox.height() - TILE_SIZE) / 2;
	gl_draw_sprite(sprite, bbox.x1, bbox.y1 + y_offset, sprite_col);

	Pos pos(bbox.x1 + TILE_SIZE * 1.25, bbox.y1 + y_offset + TILE_SIZE / 2);

	using namespace ldraw;
	const Font& font = gs->font();
	font.draw_wrapped(DrawOptions().origin(LEFT_CENTER).colour(text_col), pos,
			bbox.width() - TILE_SIZE, text);

	draw_rectangle_outline(bbox_col, bbox);
}
void draw_option_box(GameState* gs, const BBox& bbox, bool option_set,
		sprite_id sprite, const char* text) {
	Colour bbox_col = option_set ? COL_WHITE : COL_FILLED_OUTLINE;
	if (bbox.contains(gs->mouse_pos())) {
		bbox_col = COL_GOLD;
	}
	draw_setting_box(gs, bbox, bbox_col, sprite,
			COL_WHITE.with_alpha(option_set ? 255 : 100), text,
			option_set ? COL_WHITE : COL_MID_GRAY);
}

void draw_speed_box(GameState* gs, const BBox& bbox) {
	GameSettings& settings = gs->game_settings();
	const char* text = "Speed: Slow ";
	if (settings.time_per_step <= 10) {
		text = "Speed: Very Fast";
	} else if (settings.time_per_step <= 12) {
		text = "Speed: Fast";
	} else if (settings.time_per_step <= 14) {
		text = "Speed: Normal";
	}
	int diff = settings.time_per_step - 10;
	draw_setting_box(gs, bbox,
			bbox.contains(gs->mouse_pos()) ? COL_GOLD : COL_WHITE,
			get_sprite_by_name("speed setting icon"),
			COL_WHITE.with_alpha(255 - diff * 30), text, COL_WHITE);
}
