/*
 * TextBoxInst.cpp:
 *  Represents an interactive text-box.
 */

#include "../draw/colour_constants.h"

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

void TextBoxInst::select(GameState* gs) {
	selected = true;
}
void TextBoxInst::deselect(GameState* gs) {
	selected = false;
	deselect_callback.call(gs, this);
}

void TextBoxInst::step(GameState* gs) {
	if (gs->mouse_left_click()) {
		bool hover = hovered(gs, this);
		if (selected && !hover) {
			deselect(gs);
		} else if (hover) {
			select(gs);
		}
	}
	if (gs->key_press_state(SDLK_RETURN)) {
		deselect(gs);
	}
	if (selected) {
		text_field.step();
		std::vector<SDL_Event>& events = gs->io_controller().get_events();
		for (int i = 0; i < events.size(); i++) {
			text_field.handle_event(&events[i]);
		}
		update_callback.call(gs, this);
	} else {
		text_field.clear_keystate();
	}
}
const int BLINK_TIME_MS = 600;
const int BLINK_HELD_MS = 600;
void TextBoxInst::draw(GameState* gs) {
	Colour drawcol = COL_UNFILLED_OUTLINE;

	if (selected) {
		drawcol = COL_WHITE;
	} else if (hovered(gs, this)) {
		drawcol = COL_FILLED_OUTLINE;
	}
	gl_draw_rectangle(bbox, COL_DARKER_GRAY);
	int text_x = bbox.x1 + 4, text_y = bbox.center_y();
	Colour textcol = valid_string ? COL_MUTED_GREEN : COL_LIGHT_RED;
	Dim offset = gl_printf_y_centered(gs->primary_font(), textcol, text_x,
			text_y, "%s", text_field.text().c_str());
	if (selected && blink_timer.get_microseconds() / 1000 > BLINK_TIME_MS) {
		gl_printf_y_centered(gs->primary_font(), textcol, text_x + offset.w,
				text_y, "|");
		if (blink_timer.get_microseconds() / 1000
				> BLINK_TIME_MS + BLINK_HELD_MS) {
			blink_timer.start();
		}
	} else if (!selected) {
		blink_timer.start();
	}
	gl_draw_rectangle_outline(bbox, drawcol);
}

void TextBoxInst::copy_to(GameInst* inst) const {
	LANARTS_ASSERT(dynamic_cast<TextBoxInst*>(inst));
	*(TextBoxInst*)inst = *this;
}

TextBoxInst* TextBoxInst::clone() const {
	return new TextBoxInst(*this);
}
