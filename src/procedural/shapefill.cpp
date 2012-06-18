/*
 * shapefill.cpp:
 *  Fill portions of a generated level with a shape, affecting arbitrary
 *  properties
 */

#include "shapefill.h"

static inline Sqr emptysqr() {
	Sqr ret;
	memset(&ret, 0, sizeof(Sqr));
	return ret;
}
Sqr empty_mask() {
	return emptysqr();
}
Sqr passable_mask() {
	Sqr ret = emptysqr();
	ret.passable = true;
	return ret;
}

Sqr perimeter_mask() {
	Sqr ret = emptysqr();
	ret.perimeter = true;
	return ret;
}

Sqr corner_mask() {
	Sqr ret = emptysqr();
	ret.is_corner = true;
	return ret;
}

Sqr has_instance_mask() {
	Sqr ret = emptysqr();
	ret.has_instance = true;
	return ret;
}

Sqr near_entrance_mask() {
	Sqr ret = emptysqr();
	ret.near_entrance = true;
	return ret;
}

Sqr feature_mask() {
	Sqr ret = emptysqr();
	ret.feature = (short)(-1);
	return ret;
}

Sqr groupID_mask() {
	Sqr ret = emptysqr();
	ret.groupID = (short)(-1);
	return ret;
}

Sqr roomID_mask() {
	Sqr ret = emptysqr();
	ret.roomID = (short)(-1);
	return ret;
}

void level_rectangle_fill(GeneratedLevel& level, const BBox& region,
		const FillOption& fill) {
	FOR_EACH_BBOX(region, x, y) {
		fill.fill_sqr(level.at(x, y));
	}
}

void level_rectangle_outline(GeneratedLevel & level, const BBox & region,
		const FillOption & fill) {
}

void level_oval_fill(GeneratedLevel& level, const BBox& region,
		const FillOption& fill) {
	int cx = region.center_x(), cy = region.center_y();

	float wsqr = region.width() * region.width() / 4.0f;
	float hsqr = region.height() * region.height() / 4.0f;

	FOR_EACH_BBOX(region, x, y) {
		int dx = (x - cx), dy = (y - cy);
		if (dx * dx / wsqr + dy * dy / hsqr < 1) {
			fill.fill_sqr(level.at(x, y));
		}
	}
}

void set_passable(GeneratedLevel & level, const BBox & region,
		shapefill_callback shapefill, bool passable) {
	Sqr values = empty_mask();
	values.passable = passable;
	shapefill(level, region, FillOption(values, passable_mask()));
}

