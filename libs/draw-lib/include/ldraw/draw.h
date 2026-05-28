/*
 * draw.h:
 *  Image drawing and other drawing related utility functions
 */

#ifndef LDRAW_DRAW_H_
#define LDRAW_DRAW_H_

#include <lcommon/geometry.h>
#include "DrawOptions.h"

namespace ldraw {

class Image;

void draw_rectangle(const Colour& clr, const BBoxF& bbox);
void draw_rectangle_outline(const Colour& clr, const BBoxF& bbox,
		int linewidth = 1);

void draw_circle(const Colour& clr, const PosF& pos, float radius);
void draw_circle_outline(const Colour& clr, const PosF& pos, float radius,
		int linewidth = 1);
void draw_line(const Colour& clr, const PosF& p1, const PosF& p2,
		int linewidth = 1);
void draw_arc(const Colour& clr, const PosF& pos, float radius,
		float angle_start, float angle_end, int linewidth = 1,
		int segments = 0);
void draw_radial_gradient(const Colour& inner, const Colour& outer,
		const PosF& pos, float radius, int steps = 8);

}

#endif /* DRAW_H_ */
