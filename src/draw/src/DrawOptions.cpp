/*
 * DrawOptions.cpp:
 *  Helper struct for passing arguments to draw functions
 */

#include "DrawOptions.h"

namespace ldraw {

const DrawOrigin
		LEFT_TOP(0.0f, 0.0f),       LEFT_CENTER(0.0f, 0.5f),    LEFT_BOTTOM(0.0f,1.0f),
		CENTER_TOP(0.5f, 0.0f),     CENTER(0.5f, 0.5f),         CENTER_BOTTOM(0.5f, 1.0f),
		RIGHT_TOP(1.0f, 0.0f),      RIGHT_CENTER(1.0f, 0.5f),   RIGHT_BOTTOM(1.0f, 1.0f);


BBoxF adjusted_for_origin(const BBoxF & bbox, DrawOrigin origin) {
	SizeF size = bbox.size();
	Posf new_lefttop = adjusted_for_origin(bbox.left_top(), size, origin);
	return BBoxF(new_lefttop, size);
}

Posf adjusted_for_origin(const Posf & pos, const SizeF & size,
		DrawOrigin origin) {
	return Posf(pos.x - origin.placement_x * size.w,
				pos.y - origin.placement_y * size.h);
}

}
