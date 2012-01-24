/*
 * roomgen.h
 *
 *  Created on: Aug 6, 2011
 *      Author: 100397561
 */

#ifndef ROOMGEN_H_
#define ROOMGEN_H_

struct Sqr {
	bool passable;
	bool perimeter;//if roomID 0, perimeter of tunnel
	bool is_corner;
	unsigned short groupID;//For connectivity tests
	unsigned short roomID;//0 if not in room
};

struct ItemGenSettings{

};

struct RoomgenSettings{
	Sqr* sqrs;
	int w, h, seed;
	int room_number;
	int padding;
	int max_tunnels;
	int min_room_size, max_room_size;
	RoomgenSettings(int w, int h, int seed = -1, int room_number = 50, int padding = 2,
			int max_tunnels = 6, int minr = 4, int maxr = 16) :
		sqrs(new Sqr[w*h]), w(w), h(h), seed(seed),
		room_number(room_number), padding(padding), max_tunnels(max_tunnels),
		min_room_size(minr), max_room_size(maxr){
	}
	~RoomgenSettings(){
		delete[] sqrs;
	}
};

void generate_random_level(RoomgenSettings& rs);
void random_location(RoomgenSettings& rs, int& x, int& y);

#endif /* ROOMGEN_H_ */
