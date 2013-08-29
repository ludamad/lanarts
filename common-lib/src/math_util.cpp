/*
 * math.cpp:
 *  Math utility functions for lcommon
 */

#include <algorithm>

#include <cmath>
#include "math_util.h"

#include "geometry.h"

int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

int power_of_two_round(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
}

float compute_direction(const PosF & p1, const PosF & p2) {
	return atan2(p2.y - p1.y, p2.x - p1.x);
}

int round_to_multiple(int num, int mult, bool centered) {
	return num / mult * mult + (centered ? mult / 2 : 0);
}

int centered_multiple(int num, int mult) {
	return num * mult + mult / 2;
}

Pos centered_multiple(const Pos& pos, int mult) {
	return Pos(centered_multiple(pos.x, mult), centered_multiple(pos.y, mult));
}

Pos round_to_multiple(const Pos& pos, int mult, bool centered) {
	return Pos(round_to_multiple(pos.x, mult, centered),
			round_to_multiple(pos.y, mult, centered));
}

void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry,
		float speed) {
	rx = b.x - a.x;
	ry = b.y - a.y;
	float mag = sqrt(rx * rx + ry * ry);
	if (mag > 0) {
		rx /= mag / speed;
		ry /= mag / speed;
	}
}

float distance_between(const Pos & a, const Pos & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}

float magnitude(float x, float y) {
	return sqrt(x * x + y * y);
}

void normalize(float& x, float& y, float mag) {
	float curr_mag = magnitude(x, y);
	if (curr_mag == 0.0f)
		return;
	x *= mag / curr_mag;
	y *= mag / curr_mag;
}

int signum(int val) {
	if (val > 0) return +1;
	if (val < 0) return -1;
	return 0;
}

/* Integer division, but rounding up instead of down */
int round_up_divide(int value, int divisor) {
	return (value + divisor - 1) / divisor;
}

bool circle_line_test(Pos circle_xy, int circle_rad, Pos line_start_xy,
		Pos line_end_xy) {
	Pos linedelta = line_end_xy - line_start_xy;
	Pos circledelta = line_start_xy - circle_xy;
	int ddist = linedelta.x * linedelta.x + linedelta.y * linedelta.y;
	int t = -(circledelta.x * linedelta.x + circledelta.y * linedelta.y);

	/* Restrict t to within the limits of the line segment */
	if (t < 0)
		t = 0;
	else if (t > ddist)
		t = ddist;

	int dx = (line_start_xy.x + t * linedelta.x / ddist) - circle_xy.x;
	int dy = (line_start_xy.y + t * linedelta.y / ddist) - circle_xy.y;
	int rt = (dx * dx) + (dy * dy);
	return rt < (circle_rad * circle_rad);
}

bool circle_rectangle_test(Pos circle_xy, int circle_rad, BBox rect) {
	Pos rad_duped = Pos(circle_rad, circle_rad);
	if (rect.contains(circle_xy + rad_duped) || rect.contains(circle_xy - rad_duped)) {
		return true;
	}
	return circle_line_test(circle_xy, circle_rad, Pos(rect.x1, rect.y1), Pos(rect.x2, rect.y1)) ||
		circle_line_test(circle_xy, circle_rad, Pos(rect.x1, rect.y1), Pos(rect.x1, rect.y2)) ||
		circle_line_test(circle_xy, circle_rad, Pos(rect.x1, rect.y2), Pos(rect.x2, rect.y2)) ||
		circle_line_test(circle_xy, circle_rad, Pos(rect.x2, rect.y1), Pos(rect.x2, rect.y2));
}
