/*
 * DrawOptions.h:
 *  Helper struct for passing arguments to draw functions
 */

#ifndef DRAWOPTIONS_H_
#define DRAWOPTIONS_H_

#include <common/geometry.h>

#include "Colour.h"

namespace ldraw {

enum DrawOrigin {
	LEFT_TOP = 0,
	LEFT_CENTER = 1,
	LEFT_BOTTOM = 2,
	CENTER_TOP = 3,
	CENTER = 4,
	CENTER_BOTTOM = 5,
	RIGHT_TOP = 6,
	RIGHT_CENTER = 7,
	RIGHT_BOTTOM = 8
};

//Assumes left-top origin bbox
BBoxF adjusted_for_origin(const BBoxF& bbox, DrawOrigin origin);

struct DrawOptions {
	DrawOptions(DrawOrigin origin = LEFT_TOP) :
			draw_origin(origin) {
	}
	DrawOptions(const Colour& draw_colour) :
			draw_origin(LEFT_TOP), draw_colour(draw_colour) {
	}

	DrawOptions(const BBoxF& draw_region) :
			draw_origin(LEFT_TOP), draw_region(draw_region) {
	}

	DrawOptions(DrawOrigin origin, const Colour& draw_colour,
			const BBoxF& draw_region) :
			draw_origin(origin), draw_colour(draw_colour), draw_region(
					draw_region) {
	}

	DrawOrigin draw_origin;
	BBoxF draw_region;
	Colour draw_colour;
};

}

#endif /* DRAWOPTIONS_H_ */
