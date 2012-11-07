/*
 * range.h:
 *  Represents a range
 */

#ifndef RANGE_H_
#define RANGE_H_

#include "lcommon_defines.h"

/*Represents a range*/
struct Range {
	int min, max;
	explicit Range(int min = 0, int max = 0) :
			min(min), max(max) {
		LCOMMON_ASSERT(min <= max);
	}
	Range multiply(int n) const {
		return Range(min * n, max * n);
	}
};

/*Represents a float range*/
struct RangeF {
	float min, max;
	explicit RangeF(float min = 0, float max = 0) :
			min(min), max(max) {
		LCOMMON_ASSERT(min <= max);
	}

	explicit RangeF(const Range& r) :
			min(r.min), max(r.max) {
		LCOMMON_ASSERT(min <= max);
	}
	Range multiply(float n) const {
		return Range(min * n, max * n);
	}
};

#endif /* RANGE_H_ */
