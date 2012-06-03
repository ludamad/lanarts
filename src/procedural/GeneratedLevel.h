/*
 * GeneratedLevel.h
 * 	Utility class for room generation functions, stores generated room state & where the rooms are located
 */

#ifndef GENERATEDLEVEL_H_
#define GENERATEDLEVEL_H_
#include <cstring>
#include <vector>
#include "../util/mtwist.h"
#include "../util/game_basic_structs.h"

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
	bool near_entrance;
	feature_t feature;
	unsigned short groupID; //For connectivity tests
	unsigned short roomID; //0 if not in room
	Sqr(int passable, int perimeter, int is_corner, feature_t f, int groupID, int roomID) :
			passable(passable), perimeter(perimeter), is_corner(is_corner), has_instance(false),
			near_entrance(false), feature(
					f), groupID(groupID), roomID(roomID) {
	}
	Sqr() {
	}
};


struct Room {
	enum shape_t {
		RECT = 0,
		OVAL = 1
	};

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
		s = NULL;
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
	void set_circle_with_perimeter(const Region & r, const Sqr & val,
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
