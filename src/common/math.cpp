/*
 * math.cpp:
 *  Math utility functions for lcommon
 */

#include "math.h"

#include "geometry.h"

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

