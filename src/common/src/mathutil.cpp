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
