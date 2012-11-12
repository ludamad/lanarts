/*
 * draw.h:
 *  Image drawing and other drawing related utility functions
 */

#ifndef DRAW_H_
#define DRAW_H_

#include <common/geometry.h>
#include "DrawOptions.h"

namespace ldraw {

class Image;

void draw_rectangle(const Colour& clr, const BBoxF& bbox);
void draw_rectangle_outline(const Colour& clr, const BBoxF& bbox,
		int linewidth = 1);

void draw_circle(const Colour& clr, const Posf& pos, float radius);
void draw_circle_outline(const Colour& clr, const Posf& pos, float radius,
		int linewidth = 1);
}

#endif /* DRAW_H_ */
