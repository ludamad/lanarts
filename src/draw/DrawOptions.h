/*
 * DrawOptions.h:
 *  Helper struct for passing arguments to draw functions
 */

#ifndef DRAWOPTIONS_H_
#define DRAWOPTIONS_H_

#include <common/geometry.h>

#include "Colour.h"

namespace ldraw {

struct DrawOptions {
	DrawOptions() {
	}
	DrawOptions(const Colour& draw_colour) :
			draw_colour(draw_colour) {
	}

	BBoxF draw_region;
	Colour draw_colour;
};

}

#endif /* DRAWOPTIONS_H_ */
