/*
 * math_util.h:
 *  Defines utility functions for things such as vector normalization, rounding to powers of two, etc
 */

#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include <lcommon/mathutil.h>

#include "lanarts_defines.h"

struct Pos;

// Using these is now deprecated, please use lcommon
// And move things there as necessary
#pragma deprecated
void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry,
		float speed = 1);
#pragma deprecated
float distance_between(const Pos& a, const Pos& b);
#pragma deprecated
float magnitude(float x, float y);
#pragma deprecated
void normalize(float& x, float& y, float mag = 1);

#endif /* MATH_UTIL_H_ */
