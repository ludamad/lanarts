/*
 * DrawOptions.h:
 *  Helper struct for passing arguments to draw functions
 */

#ifndef DRAWOPTIONS_H_
#define DRAWOPTIONS_H_

#include <lcommon/geometry.h>

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
Posf adjusted_for_origin(const Posf& pos, const DimF& size, DrawOrigin origin);

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
			Colour(), const DimF& draw_scale = DimF(1.0f, 1.0f),
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

	DrawOptions& scale(const DimF& draw_scale) {
		this->draw_scale = draw_scale;
		return *this;
	}

	DrawOptions& region(const BBoxF& draw_region) {
		this->draw_region = draw_region;
		return *this;
	}

	DrawOrigin draw_origin;
	Colour draw_colour;
	DimF draw_scale;
	BBoxF draw_region;
	float draw_angle;
	float draw_frame;
};

}

#endif /* DRAWOPTIONS_H_ */
