/*
 * TextBoxInst.cpp:
 *  Represents an interactive text-box.
 */

#include "../display/colour_constants.h"

#include "../gamestate/GameState.h"

#include "TextBoxInst.h"

static bool hovered(GameState* gs, TextBoxInst* button) {
	BBox tbounds(button->bounds(gs));
	return tbounds.contains(gs->mouse_pos());
}

BBox TextBoxInst::bounds(GameState* gs) {
	return bbox;
}

void TextBoxInst::init(GameState* gs) {
}

void TextBoxInst::step(GameState* gs) {
	if (gs->mouse_left_click()) {
		selected = hovered(gs, this);
	}
	if (gs->key_press_state(SDLK_RETURN)) {
		selected = false;
	}
	if (selected) {
		text_field.step();
		std::vector<SDL_Event>& events = gs->io_controller().get_events();
		for (int i = 0; i < events.size(); i++) {
			text_field.handle_event(&events[i]);
		}
	} else {
		text_field.clear_keystate();
	}
}

void TextBoxInst::draw(GameState* gs) {
	Colour drawcol = COL_UNFILLED_OUTLINE;
	if (selected) {
		drawcol = COL_WHITE;
	} else if (hovered(gs, this)) {
		drawcol = COL_FILLED_OUTLINE;
	}
	gl_draw_rectangle(bbox, Colour(0, 0, 0));
	gl_printf_y_centered(gs->primary_font(), COL_WHITE, bbox.x1,
			bbox.center_y(), "%s", text_field.text().c_str());
	gl_draw_rectangle_outline(bbox, drawcol);
}

void TextBoxInst::copy_to(GameInst* inst) const {
	LANARTS_ASSERT(dynamic_cast<TextBoxInst*>(inst));
	*(TextBoxInst*)inst = *this;
}

TextBoxInst* TextBoxInst::clone() const {
	return new TextBoxInst(*this);
}

