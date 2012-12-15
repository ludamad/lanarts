/*
 * draw_statbar.cpp:
 *  Function to draw a simple stat bar, consisting of two rectangles.
 */

#include <ldraw/Colour.h>
#include <ldraw/draw.h>
#include <lcommon/geometry.h>

#include "display/display.h"
#include "draw_statbar.h"

void draw_statbar(const BBoxF& bbox, float fill, const Colour& front,
		const Colour& back) {
	ldraw::draw_rectangle(back, bbox);
	ldraw::draw_rectangle(front,
			BBoxF(bbox.left_top(), DimF(bbox.width() * fill, bbox.height())));
}

void draw_statbar(const BBoxF & bbox, float fill) {
	draw_statbar(bbox, fill, Colour(0, 255, 0), Colour(255, 0, 0));
}

