/*
 * colour_constants.h:
 * 	Various constants for colours used through-out the program.
 */

#ifndef COLOUR_CONSTANTS_H_
#define COLOUR_CONSTANTS_H_

#include "../lanarts_defines.h"

/* Colours */
const Colour COL_YELLOW(255, 255, 0);
const Colour COL_MUTED_YELLOW(255, 250, 70);
const Colour COL_PALE_YELLOW(255, 250, 150);

const Colour COL_LIGHT_RED(255, 50, 50);
const Colour COL_RED(255, 0, 0);
const Colour COL_MUTED_RED(255, 50, 50);
const Colour COL_PALE_RED(255, 148, 120);

const Colour COL_MUTED_GREEN(50, 255, 50);
const Colour COL_PALE_GREEN(180, 255, 180);
const Colour COL_GREEN(0, 255, 0);

const Colour COL_BLUE(0, 0, 255);
const Colour COL_PALE_BLUE(180, 180, 255);
const Colour COL_BABY_BLUE(37, 207, 240);

const Colour COL_GOLD(255, 215, 0);

const Colour COL_MEDIUM_PURPLE(123, 104, 238);

const Colour COL_WHITE(255, 255, 255);
const Colour COL_BLACK(0, 0, 0);
const Colour COL_DARKER_GRAY(20, 20, 20);
const Colour COL_DARK_GRAY(40, 40, 40);
const Colour COL_GRAY(60, 60, 60);
const Colour COL_LIGHT_GRAY(180, 180, 180);
const Colour COL_MID_GRAY(120, 120, 120);

/* Text console constants */
const Colour COL_CONSOLE_BOX(COL_PALE_BLUE);

/* For drawing of the action bar and inventory */
const Colour COL_SELECTED_OUTLINE(50, 205, 50);
const Colour COL_FILLED_OUTLINE(120, 115, 110);
const Colour COL_UNFILLED_OUTLINE(43, 43, 43);

/* Sidebar constants */
const Colour COL_HOVER_VIEWICON(COL_PALE_YELLOW);
const Colour COL_SELECTED_VIEWICON(COL_GOLD);

#endif /* COLOUR_CONSTANTS_H_ */
