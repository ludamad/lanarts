/*
 * math_util.h:
 *  Defines utility functions for things such as vector normalization, rounding to powers of two, etc
 */

#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include "../lanarts_defines.h"

struct Pos;

int power_of_two(int input);
void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry,
		float speed = 1);
int squish(int a, int b, int c);
float distance_between(const Pos& a, const Pos& b);
float magnitude(float x, float y);
void normalize(float& x, float& y, float mag = 1);

int round_to_multiple(int num, int mult, bool centered = false);
int centered_multiple(int num, int mult);
Pos centered_multiple(const Pos& pos, int mult);
Pos round_to_multiple(const Pos& pos, int mult, bool centered = false);

#endif /* MATH_UTIL_H_ */
