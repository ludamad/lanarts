/*
 * roomgen.h
 *
 *  Created on: Aug 6, 2011
 *      Author: 100397561
 */

#ifndef ROOMGEN_H_
#define ROOMGEN_H_

enum {
	NORMAL = 1,
	SMALL_CORRIDOR = 2,
	LARGE_CORRIDOR = 3,
	DOOR = 4

};
struct Sqr {
	bool passable;
	bool perimeter;//if roomID 0, perimeter of tunnel
	bool is_corner;
	unsigned char marking;
	unsigned short groupID;//For connectivity tests
	unsigned short roomID;//0 if not in room
};

struct ItemGenSettings{

};

struct RoomgenSettings{
	Sqr* sqrs;
	ItemGenSettings itemgen;
	int w, h, seed;
	int room_number;
	int padding;
	int max_tunnels;
	int min_room_size, max_room_size;
	RoomgenSettings(int w, int h, int seed = -1, int room_number = 90, int padding = 1,
			int max_tunnels = 20, int minr = 4, int maxr = 30) :
		sqrs(new Sqr[w*h]), itemgen(), w(w), h(h), seed(seed),
		room_number(room_number), padding(padding), max_tunnels(max_tunnels),
		min_room_size(minr), max_room_size(maxr){
	}
	~RoomgenSettings(){
		delete[] sqrs;
	}
};

void generate_random_level(RoomgenSettings& rs);
void random_location(RoomgenSettings& rs, int seed, int& x, int& y);

#endif /* ROOMGEN_H_ */
