/*
 * math_util.cpp:
 *  Defines utility functions for things such as vector normalization, rounding to powers of two, etc
 */

#include <cmath>
#include <algorithm>

#include "math_util.h"

int power_of_two(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
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

int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

float distance_between(const Pos & a, const Pos & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}

int round_to_multiple(int num, int mult, bool centered) {
	return num / mult * mult + (centered ? mult / 2 : 0);
}

int centered_multiple(int num, int mult) {
	return num * mult + mult / 2;
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

Pos centered_multiple(const Pos& pos, int mult) {
	return Pos(centered_multiple(pos.x, mult), centered_multiple(pos.y, mult));
}

Pos round_to_multiple(const Pos& pos, int mult, bool centered) {
	return Pos(round_to_multiple(pos.x, mult, centered),
			round_to_multiple(pos.y, mult, centered));
}

