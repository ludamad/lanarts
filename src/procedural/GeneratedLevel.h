/*
 * GeneratedLevel.h
 * 	Utility class for room generation functions
 */

#ifndef GENERATEDLEVEL_H_
#define GENERATEDLEVEL_H_
#include <cstring>
#include <vector>
#include "mtwist.h"

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
	bool passable;
	bool perimeter; //if roomID 0, perimeter of tunnel
	bool is_corner;
	bool has_instance;
	feature_t feature;
	unsigned short groupID; //For connectivity tests
	unsigned short roomID; //0 if not in room
	Sqr(int passable, int perimeter, int is_corner, feature_t f, int groupID, int roomID) :
			passable(passable), perimeter(perimeter), is_corner(is_corner), has_instance(false), feature(
					f), groupID(groupID), roomID(roomID) {
	}
	Sqr() {
	}
};

struct Pos {
	int x, y;
	Pos() {}
	bool operator==(const Pos& o) const {
		return o.x == x && o.y == y;
	}
	Pos(int x, int y) :
			x(x), y(y) {
	}
};

struct Region {
	int x, y, w, h;
	Region(int x, int y, int w, int h) :
			x(x), y(y), w(w), h(h) {
	}
	inline Pos pos() {
		return Pos(x, y);
	}
};

struct Room {
	Region room_region;
	int groupID;
	Room(const Region& r, int groupID) :
		room_region(r), groupID(groupID){
	}
	void operator=(const Room& r){
		memcpy(this, &r, sizeof(Room));
	}
};

class GeneratedLevel {
public:
	int width() { return w; }
	int height() { return h; }
	std::vector<Room>& rooms(){ return room_list; }

	GeneratedLevel() {
		s = NULL;
		w = 0, h = 0;
	}
	void initialize(int w, int h){
		delete[] s;
		this->w = w;
		this->h = h;
		s = new Sqr[w * h];
		memset(s, 0, sizeof(Sqr) * w * h);
	}
	GeneratedLevel(int w, int h) {
		initialize(w,h);
	}
	~GeneratedLevel() {
		delete[] s;
	}

	Sqr& at(int x, int y);
	Sqr& at(const Pos & p);

	void set_region(const Region& r, const Sqr& val);
	void set_region_corners(const Region & r);
	void set_region_with_perimeter(const Region & r, const Sqr & val,
			int padding);

	bool verify(const Region& r, bool check_init = true);
	int region_groupID(const Region& r, Pos& p);

private:
	Sqr* s;
	int w, h;
	std::vector<Room> room_list;
};

Pos generate_location(MTwist& mt, GeneratedLevel& level);
Pos generate_location_byroom(MTwist& mt, GeneratedLevel& level);
Pos generate_location_in_region(MTwist& mt, GeneratedLevel& level, const Region& r);

#endif /* GENERATEDROOM_H_ */
