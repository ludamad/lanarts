/*
 * fonts.cpp:
 *  Accessors for the fonts used by the game.
 */

#include "fonts.h"

using namespace ldraw;

static Font __primary, __menu;

namespace res {

Font& font_primary() {
	return __primary;
}
Font& font_menu() {
	return __menu;
}


void font_free() {
	__primary.clear();
	__menu.clear();
}

}
