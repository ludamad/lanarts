/*
 * math_util.h:
 *  Defines utility functions for things such as vector normalization, rounding to powers of two, etc
 */

#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include "../lanarts_defines.h"

struct Pos;

// Using these is now deprecated, please move use lcommon
// And move things there as necessary
#pragma deprecated
int power_of_two(int input);
#pragma deprecated
void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry,
		float speed = 1);
#pragma deprecated
int squish(int a, int b, int c);
#pragma deprecated
float distance_between(const Pos& a, const Pos& b);
#pragma deprecated
float magnitude(float x, float y);
#pragma deprecated
void normalize(float& x, float& y, float mag = 1);

#pragma deprecated
int round_to_multiple(int num, int mult, bool centered = false);
#pragma deprecated
int centered_multiple(int num, int mult);
#pragma deprecated
Pos centered_multiple(const Pos& pos, int mult);
#pragma deprecated
Pos round_to_multiple(const Pos& pos, int mult, bool centered = false);

#endif /* MATH_UTIL_H_ */
