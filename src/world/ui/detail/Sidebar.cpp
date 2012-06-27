/*
 * Sidebar.cpp:
 *  Handles drawing & state of side bar
 */

#include "Sidebar.h"

static BBox content_area_box(const BBox& sidebar_box) {
	const int CONTENT_AREA_Y = 342;
	const int CONTENT_ROWS = 8;
	int sx = sidebar_box.x1, sy = sidebar_box.y1 + CONTENT_AREA_Y;
	int ex = sidebar_box.x2, ey = sy + CONTENT_ROWS * TILE_SIZE;
	return BBox(sx, sy, ex, ey);
}

Sidebar::Sidebar(const BBox& sidebar_box) :
		navigator(sidebar_box, content_area_box(sidebar_box)), sidebar_box(
				sidebar_box) {
}


