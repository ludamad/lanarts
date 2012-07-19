/*
 * generated_tile.h
 *  Represents a generated tile, with bitmask-like operations
 */

#ifndef GENERATED_TILE_H_
#define GENERATED_TILE_H_

#include <cstring>

enum feature_t {
	UNSET = 0,
	NORMAL = 1,
	SMALL_CORRIDOR = 2,
	LARGE_CORRIDOR = 3,
	DOOR = 4,
	STAIR_UP = 5,
	STAIR_DOWN = 6

};

struct Sqr {
	Sqr(int passable, int perimeter, int is_corner, feature_t f, int groupID,
			int roomID) :
			passable(passable), perimeter(perimeter), is_corner(is_corner), has_instance(
					false), near_entrance(false), feature(f), groupID(groupID), roomID(
					roomID) {
	}

	Sqr() :
			feature(UNSET) {
	}

	inline void operator&=(const Sqr& o) {
		passable &= o.passable;
		perimeter &= o.perimeter;
		is_corner &= o.is_corner;
		has_instance &= o.has_instance;
		near_entrance &= o.near_entrance;
		feature &= o.feature;
		groupID &= o.groupID;
		roomID &= o.roomID;
	}
	inline void operator|=(const Sqr& o) {
		passable |= o.passable;
		perimeter |= o.perimeter;
		is_corner |= o.is_corner;
		has_instance |= o.has_instance;
		near_entrance |= o.near_entrance;
		feature |= o.feature;
		groupID |= o.groupID;
		roomID |= o.roomID;
	}
	inline Sqr operator&(const Sqr& o) const {
		Sqr ret = *this;
		ret &= o;
		return ret;
	}
	inline Sqr operator|(const Sqr& o) const {
		Sqr ret = *this;
		ret |= o;
		return ret;
	}
	inline Sqr operator~() const {
		Sqr ret;
		ret.passable = !passable;
		ret.perimeter = !perimeter;
		ret.is_corner = !is_corner;
		ret.has_instance = !has_instance;
		ret.near_entrance = !near_entrance;
		ret.feature = ~feature;
		ret.groupID = ~groupID;
		ret.roomID = ~roomID;
		return ret;
	}
	bool operator==(const Sqr& o) const {
		return memcmp(this, &o, sizeof(Sqr)) == 0;
	}

	/* Tile generation flags */
	bool passable;
	bool perimeter; //if roomID 0, perimeter of tunnel
	bool is_corner;
	bool has_instance;
	bool near_entrance;
	unsigned short feature;
	unsigned short groupID; //For connectivity tests
	unsigned short roomID; //0 if not in room
};

#endif /* GENERATED_TILE_H_ */
