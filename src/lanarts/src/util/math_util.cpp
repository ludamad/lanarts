/*
 * math_util.cpp:
 *  Defines utility functions for things such as vector normalization, rounding to powers of two, etc
 */

#include <cmath>
#include <algorithm>

#include <lcommon/geometry.h>

#include "math_util.h"

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

