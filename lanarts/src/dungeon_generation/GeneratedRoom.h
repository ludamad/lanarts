/*
 * GeneratedRoom.h
 * 	Utility class for room generation functions, stores generated room state & where the rooms are located
 */

#ifndef GENERATEDROOM_H_
#define GENERATEDROOM_H_
#include <cstring>
#include <vector>

#include "objects/GameInstRef.h"
#include <lcommon/mtwist.h>
#include "lanarts_defines.h"

#include "../region.h"

#include "generated_tile.h"

struct RoomRegion {
	enum shape_t {
		RECT = 0, OVAL = 1
	};

	Region region;
	int groupID;
	int enemies_in_room;
	RoomRegion() :
			groupID(0), enemies_in_room(0) {
	}
	RoomRegion(const Region& r, int groupID) :
			region(r), groupID(groupID), enemies_in_room(0) {
	}
	void operator=(const RoomRegion& r) {
		memcpy(this, &r, sizeof(RoomRegion));
	}
};

class GeneratedRoom {
public:
	int width() const {
		return size.w;
	}
	int height() const {
		return size.h;
	}
	std::vector<RoomRegion>& rooms() {
		return room_list;
	}
	std::vector<GameInstRef>& instances() {
		return instance_list;
	}
	void add_instance(GameInst* inst) {
		return instance_list.push_back(inst);
	}
	RoomRegion& get_room(int i) {
		return room_list.at(i);
	}

	GeneratedRoom() {
		s = NULL;
		number_of_mobs = 0;
	}

	Pos get_world_coordinate(GameState* gs, const Pos& p) const;

	void initialize(int w, int h, bool solid = true) {
		number_of_mobs = 0;
		delete[] s;
		size = Size(w, h);
		s = new Sqr[w * h];
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
	GeneratedRoom(int w, int h) {
		s = NULL;
		number_of_mobs = 0;
		initialize(w, h);
	}
	~GeneratedRoom() {
		delete[] s;
	}

	Sqr& at(int x, int y);
	Sqr& at(const Pos & p);
	bool within(const Pos& p);

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
	GeneratedRoom(const GeneratedRoom&); //DO-NOT-USE
	Sqr* s;
	Size size;
	std::vector<RoomRegion> room_list;
	std::vector<GameInstRef> instance_list;
	int number_of_mobs;
};

Pos generate_location(MTwist& mt, GeneratedRoom& level);
Pos generate_location_byroom(MTwist& mt, GeneratedRoom& level);
Pos generate_location_in_region(MTwist& mt, GeneratedRoom& level,
		const Region& r);

#endif /* GENERATEDROOM_H_ */
