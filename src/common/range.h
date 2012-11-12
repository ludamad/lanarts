/*
 * range.h:
 *  Represents a range
 */

#ifndef LCOMMON_RANGE_H_
#define LCOMMON_RANGE_H_

#include "lcommon_defines.h"

/*Represents a range*/
struct Range {
	int min, max;
	explicit Range(int min = 0, int max = 0) :
			min(min), max(max) {
		LCOMMON_ASSERT(min <= max);
	}
	bool operator==(const Range& r) const {
		return min == r.min && max == r.max;
	}
	bool operator!=(const Range& r) const {
		return !(*this == r);
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

	bool operator==(const RangeF& r) const {
		return min == r.min && max == r.max;
	}
	bool operator!=(const RangeF& r) const {
		return !(*this == r);
	}

	explicit RangeF(const Range& r) :
			min(r.min), max(r.max) {
		LCOMMON_ASSERT(min <= max);
	}
	Range multiply(float n) const {
		return Range(min * n, max * n);
	}
};

#endif /* LCOMMON_RANGE_H_ */
