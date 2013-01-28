/*
 * math.cpp:
 *  Math utility functions for lcommon
 */

#include <algorithm>

#include <cmath>
#include "mathutil.h"

#include "geometry.h"

int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

int power_of_two(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
}

float compute_direction(const Posf & p1, const Posf & p2) {
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
