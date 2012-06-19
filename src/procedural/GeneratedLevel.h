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

#include "generated_tile.h"

struct Room {
	enum shape_t {
		RECT = 0, OVAL = 1
	};

	Region room_region;
	int groupID;
	int enemies_in_room;
	Room(const Region& r, int groupID) :
			room_region(r), groupID(groupID), enemies_in_room(0) {
	}
	void operator=(const Room& r) {
		memcpy(this, &r, sizeof(Room));
	}
};

class GeneratedLevel {
public:
	int width() {
		return w;
	}
	int height() {
		return h;
	}
	std::vector<Room>& rooms() {
		return room_list;
	}

	GeneratedLevel() {
		s = NULL;
		w = 0, h = 0;
	}

	void initialize(int w, int h, bool solid = true) {
		delete[] s;
		this->w = w;
		this->h = h;
		s = new Sqr[w * h];
		memset(s, 0, sizeof(Sqr) * w * h);
		if (!solid) {
			for (int i = 0; i < w * h; i++) {
				s[i].passable = true;
			}
			for (int i = 0; i < w * h; i++) {
				s[i].passable = true;
			}
		}
	}
	GeneratedLevel(int w, int h) {
		s = NULL;
		initialize(w, h);
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
	GeneratedLevel(const GeneratedLevel&); //DO-NOT-USE
	Sqr* s;
	int w, h;
	std::vector<Room> room_list;
};

Pos generate_location(MTwist& mt, GeneratedLevel& level);
Pos generate_location_byroom(MTwist& mt, GeneratedLevel& level);
Pos generate_location_in_region(MTwist& mt, GeneratedLevel& level,
		const Region& r);

#endif /* GENERATEDROOM_H_ */
