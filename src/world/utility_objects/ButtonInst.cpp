#include "ButtonInst.h"
#include "../GameState.h"
#include "../../display/display.h"
#include <typeinfo>

void ButtonInst::init(GameState* gs) {
}

BBox ButtonInst::bounds(GameState* gs) {
	Pos dims = gl_text_dimensions(gs->menu_font(), "%s", str.c_str());
	int sx = x - dims.x / 2, sy = y - dims.y / 2;
	return BBox(sx, sy, sx + dims.x, sy + dims.y);
}

void ButtonInst::step(GameState* gs) {
	BBox bounding(bounds(gs));

	bool outofx = (gs->mouse_x() < bounding.x1 || gs->mouse_x() > bounding.x2);
	bool outofy = (gs->mouse_y() < bounding.y1 || gs->mouse_y() > bounding.y2);
	if (!outofx && !outofy && gs->mouse_left_down()) {
		click_callback.call(gs, this);
	}
}

void ButtonInst::draw(GameState* gs) {
	BBox bounding(bounds(gs));

	bool outofx = (gs->mouse_x() < bounding.x1 || gs->mouse_x() > bounding.x2);
	bool outofy = (gs->mouse_y() < bounding.y1 || gs->mouse_y() > bounding.y2);
	Colour col =
			(!outofx && !outofy) ? Colour(255, 0, 0) : Colour(255, 255, 255);
//	gl_draw_rectangle(bounding.x1, bounding.y1, bounding.width(), bounding.height(),  Colour(0,255,0));
	gl_printf_centered(gs->menu_font(), col, x, y, "%s", str.c_str());
}

void ButtonInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ButtonInst*)inst = *this;
}
ButtonInst *ButtonInst::clone() const {
	return new ButtonInst(*this);
}

