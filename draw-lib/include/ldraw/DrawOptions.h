/*
 * DrawOptions.h:
 *  Helper struct for passing arguments to draw functions
 */

#ifndef DRAWOPTIONS_H_
#define DRAWOPTIONS_H_

#include <lcommon/geometry.h>

#include "Colour.h"

namespace ldraw {

struct DrawOrigin {
	float placement_x, placement_y;

	DrawOrigin(float px = 0, float py = 0)  {
		placement_x = px;
		placement_y = py;
	}

	inline bool operator==( const DrawOrigin& o ) const {
		return o.placement_x == placement_x && o.placement_y == placement_y;
	}
};

const extern DrawOrigin
	LEFT_TOP, LEFT_CENTER, LEFT_BOTTOM,
	CENTER_TOP, CENTER, CENTER_BOTTOM,
	RIGHT_TOP, RIGHT_CENTER, RIGHT_BOTTOM;

//Assumes left-top origin bbox
BBoxF adjusted_for_origin(const BBoxF& bbox, DrawOrigin origin);
PosF adjusted_for_origin(const PosF& pos, const SizeF& size, DrawOrigin origin);

struct DrawOptions {
	DrawOptions(const Colour& draw_colour) :
			draw_origin(LEFT_TOP), draw_colour(draw_colour), draw_scale(1.0f,
					1.0f), draw_angle(0.0f), draw_frame(0.0f) {
	}

	DrawOptions(const BBoxF& draw_region) :
			draw_origin(LEFT_TOP), draw_scale(1.0f, 1.0f), draw_region(
					draw_region), draw_angle(0.0f), draw_frame(0.0f) {
	}

	DrawOptions(DrawOrigin origin = LEFT_TOP, const Colour& draw_colour =
			Colour(), const SizeF& draw_scale = SizeF(1.0f, 1.0f),
			const BBoxF& draw_region = BBoxF(), float angle = 0.0f,
			float frame = 0.0f) :
			draw_origin(origin), draw_colour(draw_colour), draw_scale(
					draw_scale), draw_region(draw_region), draw_angle(angle), draw_frame(
					frame) {
	}

	DrawOptions& colour(const Colour& draw_colour) {
		this->draw_colour = draw_colour;
		return *this;
	}

	DrawOptions& frame(float draw_frame) {
		this->draw_frame = draw_frame;
		return *this;
	}
	DrawOptions& origin(DrawOrigin draw_origin) {
		this->draw_origin = draw_origin;
		return *this;
	}

	DrawOptions& angle(float draw_angle) {
		this->draw_angle = draw_angle;
		return *this;
	}

	DrawOptions& scale(const SizeF& draw_scale) {
		this->draw_scale = draw_scale;
		return *this;
	}

	DrawOptions& region(const BBoxF& draw_region) {
		this->draw_region = draw_region;
		return *this;
	}

	DrawOrigin draw_origin;
	Colour draw_colour;
	SizeF draw_scale;
	BBoxF draw_region;
	float draw_angle;
	float draw_frame;
};

}

#endif /* DRAWOPTIONS_H_ */
