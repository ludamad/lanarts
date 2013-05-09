/*
 * fonts.h:
 *  Accessors for the fonts used by the game.
 */

#ifndef FONTS_H_
#define FONTS_H_

#include <ldraw/Font.h>

namespace res {

ldraw::Font& font_primary();
ldraw::Font& font_menu();

// Free all fonts
void font_free();

}

#endif /* FONTS_H_ */
