/*
 * colour_constants.h:
 * 	Various constants for colours used through-out the program.
 */

#ifndef COLOUR_CONSTANTS_H_
#define COLOUR_CONSTANTS_H_

#include "game_basic_structs.h"

/* Colours */
const Colour YELLOW(255, 255, 0);
const Colour RED(255, 50, 50);
const Colour GOLD(255, 215, 0);
const Colour PALE_YELLOW(255, 250, 150);
const Colour MEDIUM_PURPLE(123,104,238);

/* For drawing of the action bar and inventory */
const Colour COL_SELECTED_OUTLINE(50, 205, 50);
const Colour COL_FILLED_OUTLINE(120, 115, 110);
const Colour COL_UNFILLED_OUTLINE(43, 43, 43);

/* Sidebar constants */
const Colour COL_HOVER_VIEWICON(PALE_YELLOW);
const Colour COL_SELECTED_VIEWICON(GOLD);

#endif /* COLOUR_CONSTANTS_H_ */
