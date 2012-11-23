/*
 * GeneratedLevel.h
 * 	Utility class for room generation functions, stores generated room state & where the rooms are located
 */

#ifndef GENERATEDLEVEL_H_
#define GENERATEDLEVEL_H_
#include <cstring>
#include <vector>

#include "../objects/GameInstRef.h"
#include <common/random/mtwist.h>
#include "../lanarts_defines.h"

#include "../region.h"

#include "generated_tile.h"

struct Room {
	enum shape_t {
		RECT = 0, OVAL = 1
	};

	Region room_region;
	int groupID;
	int enemies_in_room;
	Room() :
			groupID(0), enemies_in_room(0) {
	}
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
		return size.w;
	}
	int height() {
		return size.h;
	}
	std::vector<Room>& rooms() {
		return room_list;
	}
	std::vector<GameInstRef>& instances() {
		return instance_list;
	}
	void add_instance(GameInst* inst) {
		return instance_list.push_back(inst);
	}
	Room& get_room(int i) {
		return room_list.at(i);
	}

	GeneratedLevel() {
		s = NULL;
		number_of_mobs = 0;
	}

	Pos get_world_coordinate(const Pos& p);

	void initialize(int w, int h, const Pos& offset, bool solid = true) {
		number_of_mobs = 0;
		delete[] s;
		size = Dim(w, h);
		s = new Sqr[w * h];
		world_offset = offset;
		memset(s, 0, sizeof(Sqr) * w * h);
		if (!solid) {
			for (int i = 0; i < w * h; i++) {
				s[i].passable = true;
			}
		} else {
			for (int i = 0; i < w * h; i++) {
				s[i].passable = false;
			}
		}
	}
	GeneratedLevel(int w, int h, const Pos& offset) {
		s = NULL;
		number_of_mobs = 0;
		initialize(w, h, offset);
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

	int get_next_mob_id() {
		return number_of_mobs++;
	}

private:
	GeneratedLevel(const GeneratedLevel&); //DO-NOT-USE
	Sqr* s;
	Dim size;
	Pos world_offset;
	std::vector<Room> room_list;
	std::vector<GameInstRef> instance_list;
	int number_of_mobs;
};

Pos generate_location(MTwist& mt, GeneratedLevel& level);
Pos generate_location_byroom(MTwist& mt, GeneratedLevel& level);
Pos generate_location_in_region(MTwist& mt, GeneratedLevel& level,
		const Region& r);

#endif /* GENERATEDROOM_H_ */
