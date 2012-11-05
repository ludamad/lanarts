#include <typeinfo>

#include "../display/display.h"
#include "../display/sprite_data.h"
#include "../gamestate/GameState.h"

#include "ButtonInst.h"

void ButtonInst::init(GameState* gs) {
}

BBox ButtonInst::sprite_bounds(GameState* gs) {
	if (sprite > -1) {
		SpriteEntry& spr_entry = game_sprite_data.at(sprite);
		Dim dims = spr_entry.size();
		int sx = x - dims.w / 2, sy = y - dims.h / 2;
		return BBox(sx, sy, sx + dims.w, sy + dims.h);
	} else {
		return BBox();
	}
}

BBox ButtonInst::text_bounds(GameState* gs) {
	Dim dims = gl_text_dimensions(gs->menu_font(), "%s", str.c_str());
	int sx = x - dims.w / 2, sy = y - dims.h / 2;
	if (sprite > -1) {
		sy += sprite_bounds(gs).height();
		sy -= dims.h;
	}

	return BBox(sx, sy, sx + dims.w, sy + dims.h);
}

static bool hovered(GameState* gs, ButtonInst* button) {
	BBox tbounds(button->text_bounds(gs));
	BBox sbounds(button->sprite_bounds(gs));
	return tbounds.contains(gs->mouse_pos())
			|| sbounds.contains(gs->mouse_pos());
}

void ButtonInst::step(GameState* gs) {
	if (hovered(gs, this) && gs->mouse_left_click()) {
		click_callback.call(gs, this);
	}
}

void ButtonInst::draw(GameState* gs) {
	draw_callback.call(gs, this);
	BBox tbounds(text_bounds(gs));
	BBox sbounds(sprite_bounds(gs));
	Colour col = (hovered(gs, this)) ? _hover_colour : _draw_colour;
	if (sprite > -1) {
		gl_draw_sprite(sprite, sbounds.x1, sbounds.y1, col);
	}
	gl_printf_centered(gs->menu_font(), col, tbounds.center_x(),
			tbounds.center_y(), "%s", str.c_str());
}

void ButtonInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ButtonInst*)inst = *this;
}
ButtonInst *ButtonInst::clone() const {
	return new ButtonInst(*this);
}

