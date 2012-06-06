#include "ButtonInst.h"
#include "../GameState.h"
#include "../../display/display.h"
#include <typeinfo>

void ButtonInst::init(GameState* gs) {
}

void ButtonInst::step(GameState* gs) {

	bool outofx = (gs->mouse_x() < bounding.x1 || gs->mouse_x() > bounding.x2);
	bool outofy = (gs->mouse_y() < bounding.y1 || gs->mouse_y() > bounding.y2);
	if (!outofx && !outofy && gs->mouse_left_down()) {
		if (callback)
			callback(data);
	}
}

void ButtonInst::draw(GameState* gs) {
	bool outofx = (gs->mouse_x() < bounding.x1 || gs->mouse_x() > bounding.x2);
	bool outofy = (gs->mouse_y() < bounding.y1 || gs->mouse_y() > bounding.y2);
	Colour col =
			(!outofx && !outofy) ? Colour(255, 0, 0) : Colour(255, 255, 255);
//	gl_draw_rectangle(bounding.x1, bounding.y1, bounding.width(), bounding.height(),  Colour(0,255,0));
	gl_printf(gs->menu_font(), col, bounding.x1, bounding.y1, "%s", str.c_str());
}

void ButtonInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ButtonInst*)inst = *this;
}

ButtonInst *ButtonInst::clone() const {
	return new ButtonInst(*this);
}

