/*
 * DrawOptions.cpp:
 *  Helper struct for passing arguments to draw functions
 */

#include "DrawOptions.h"

namespace ldraw {

BBoxF adjusted_for_origin(const BBoxF & bbox, DrawOrigin origin) {
	float w = bbox.width(), h = bbox.height();
	float xtranslate = 0, ytranslate = 0;

	if (origin >= CENTER_TOP && origin <= CENTER_BOTTOM) {
		xtranslate = -w / 2;
	} else if (origin >= RIGHT_TOP && origin <= RIGHT_BOTTOM) {
		xtranslate = -w;
	}

	if (origin == LEFT_CENTER || origin == CENTER || origin == RIGHT_CENTER) {
		ytranslate = -h / 2;
	} else if (origin == LEFT_BOTTOM || origin == CENTER_BOTTOM
			|| origin == RIGHT_BOTTOM) {
		ytranslate = -h;
	}
	return bbox.translated(xtranslate, ytranslate);
}

}
