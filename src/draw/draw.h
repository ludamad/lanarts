/*
 * draw.h:
 *  Image drawing and other drawing related utility functions
 */

#ifndef LDRAW_DRAW_H_
#define LDRAW_DRAW_H_

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
void draw_line(const Colour& clr, const Posf& p1, const Posf& p2,
		int linewidth = 1);

}

#endif /* DRAW_H_ */
