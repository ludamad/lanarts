/*
 * math.h:
 *  Math utility functions for lcommon
 */

#ifndef LCOMMON_MATH_H_
#define LCOMMON_MATH_H_

#include <math.h>

struct Posf;

int power_of_two(int input);
float compute_direction(const Posf& p1, const Posf& p2);

const float FLOAT_PI = 3.14159265358979323846f;

#ifdef __GNUC__
//inline long int round2int(float f) {
//	return lrintf(f);
//}
//for now:
inline long int round2int(float flt) {
	return (long int)round(flt);
}
#elif (defined (WIN32) || defined (_WIN32))
__inline long int round2int (float flt) {
	int intgr;
	_asm
	{	fld flt
		fistp intgr
	};
	return intgr;
}
#endif

#endif /* LCOMMON_MATH_H_ */
