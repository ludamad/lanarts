#include <cstdio>
#include <cmath>

#include "draw.h"
#include "ldraw_assert.h"

void ldraw::draw_rectangle(const Colour& clr, const BBoxF& bbox) {
}

void ldraw::draw_rectangle_outline(const Colour& clr, const BBoxF& bbox,
		int linewidth) {
}
const float PI = 3.141592f;
const float DEG2RAD = PI / 180.0f;

static void gl_draw_circle(const Colour& clr, const PosF& pos, float radius,
		bool outline) {
}

void ldraw::draw_circle(const Colour& clr, const PosF& pos, float radius) {
	gl_draw_circle(clr, pos, radius, false);
}

void ldraw::draw_circle_outline(const Colour& clr, const PosF& pos,
		float radius, int linewidth) {
}

void ldraw::draw_line(const Colour& clr, const PosF& p1, const PosF& p2,
		int linewidth) {
}

void ldraw::draw_arc(const Colour& clr, const PosF& pos, float radius,
		float angle_start, float angle_end, int linewidth, int segments) {
}

void ldraw::draw_radial_gradient(const Colour& inner, const Colour& outer,
		const PosF& pos, float radius, int steps) {
}
