/*
 * fonts.cpp:
 *  Accessors for the fonts used by the game.
 */

#include "fonts.h"

using namespace ldraw;

static Font __primary, __bigprimary, __menu;

namespace res {

Font& font_primary() {
	return __primary;
}
Font& font_bigprimary() {
	return __bigprimary;
}
Font& font_menu() {
	return __menu;
}


void font_free() {
	__bigprimary.clear();
	__primary.clear();
	__menu.clear();
}

}
