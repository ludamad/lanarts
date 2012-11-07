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
void draw_circle(const Colour& clr, const Posf& pos, float radius,
		bool outline = false);

}

#endif /* DRAW_H_ */
