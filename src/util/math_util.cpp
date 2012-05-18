/*
 * math_util.cpp
 *
 *  Created on: Apr 25, 2012
 *      Author: 100397561
 */

#include "math_util.h"
#include <cmath>
#include <algorithm>

int power_of_two(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
}

void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry, float speed){
	rx = b.x - a.x;
	ry = b.y - a.y;
	float mag = sqrt(rx*rx + ry*ry);
	if (mag > 0){
		rx /= mag/speed;
		ry /= mag/speed;
	}
}

int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

float distance_between(const Pos & a, const Pos & b){
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx*dx + dy*dy);
}


