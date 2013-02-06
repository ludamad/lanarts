#include <typeinfo>

#include <ldraw/DrawOptions.h>
#include <ldraw/Font.h>

#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"
#include "gamestate/GameState.h"

#include "ButtonInst.h"

void ButtonInst::init(GameState* gs) {
}

BBox ButtonInst::sprite_bounds(GameState* gs) {
	if (sprite > -1) {
		SpriteEntry& spr_entry = game_sprite_data.at(sprite);
		Size size = spr_entry.size();
		int sx = x - size.w / 2, sy = y - size.h / 2;
		return BBox(sx, sy, sx + size.w, sy + size.h);
	} else {
		return BBox();
	}
}

BBox ButtonInst::text_bounds(GameState* gs) {
	SizeF size = gs->menu_font().get_draw_size(str);
	int sx = x - size.w / 2, sy = y - size.h / 2;
	if (sprite > -1) {
		sy += sprite_bounds(gs).height();
		sy -= size.h;
	}

	return BBox(sx, sy, sx + size.w, sy + size.h);
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
	BBox sbounds(sprite_bounds(gs));
	Colour col = (hovered(gs, this)) ? _hover_colour : _draw_colour;
	if (sprite > -1) {
		draw_sprite(sprite, sbounds.x1, sbounds.y1, col);
	}
	using namespace ldraw;
	gs->menu_font().draw(DrawOptions(col).origin(CENTER),
			text_bounds(gs).center(), str);
}

void ButtonInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ButtonInst*)inst = *this;
}
ButtonInst *ButtonInst::clone() const {
	return new ButtonInst(*this);
}

