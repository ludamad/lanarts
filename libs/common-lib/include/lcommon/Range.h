/*
 * Range.h:
 *  Represents a range
 */

#ifndef LCOMMON_RANGE_H_
#define LCOMMON_RANGE_H_

#include "lcommon_assert.h"

/*Represents a range*/
struct Range {
	int min, max;
	Range() : min(0), max(0) {
	}
	explicit Range(int min, int max) :
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

	RangeF() : min(0.0f), max(0.0f) {
	}
	explicit RangeF(float min, float max) :
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
			min((float)r.min), max((float)r.max) {
		LCOMMON_ASSERT(min <= max);
	}
	RangeF multiply(float n) const {
		return RangeF(min * n, max * n);
	}
};

#endif /* LCOMMON_RANGE_H_ */
