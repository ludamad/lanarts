/*
 * math.h:
 *  Math utility functions for lcommon
 */

#ifndef LCOMMON_MATH_UTIL_H_
#define LCOMMON_MATH_UTIL_H_

#include <cmath>

#include "geometry.h"

int power_of_two_round(int input);
float compute_direction(const PosF& p1, const PosF& p2);

const float FLOAT_PI = 3.14159265358979323846f;

#ifdef __GNUC__
inline int iround(float f) {
	return (int)lrintf(f);
}
#elif (defined (WIN32) || defined (_WIN32))
__inline int iround (float flt) {
	int intgr;
	_asm
	{	fld flt
		fistp intgr
	};
	return intgr;
}
#endif

/* Returns -1, 0, or +1 depending on sign */
int signum(int val);

int squish(int a, int b, int c);

Pos centered_multiple(const Pos& pos, int mult);
Pos round_to_multiple(const Pos& pos, int mult, bool centered = false);

int round_to_multiple(int num, int mult, bool centered = false);
int centered_multiple(int num, int mult);

void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry,
		float speed = 1);
float distance_between(const Pos& a, const Pos& b);
float magnitude(float x, float y);
void normalize(float& x, float& y, float mag = 1);

/* Integer division, but rounding up instead of down */
int round_up_divide(int value, int divisor);

bool circle_line_test(Pos circle_xy, int circle_rad, Pos line_start_xy, Pos line_end_xy);
bool circle_rectangle_test(Pos circle_xy, int circle_rad, BBox rect);
bool rectangle_line_test(BBox rect, Pos from_xy, Pos to_xy);

#endif /* LCOMMON_MATH_UTIL_H_ */
