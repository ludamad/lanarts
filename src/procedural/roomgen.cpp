/*
 * roomgen.cpp
 *
 *  Created on: Aug 6, 2011
 *      Author: 100397561
 */

#include "roomgen.h"
#include <cstring>
#include <ctime>
#include "mtwist.h"
#include <cstdio>
#include <vector>
using namespace std;

struct Pos {
	int x, y;
};
struct Region {
	int x, y, w, h;
	inline Pos pos(){
		Pos p = {x,y};
		return p;
	}
};
struct Squares {
	Sqr* s;
	int w, h;
	inline Sqr& at(int x, int y){
		return s[y*w+x];
	}
	inline Sqr& at(const Pos& p){
		return s[p.y*w+p.x];
	}
	void set_region_corners(const Region& r){
		at(r.x, r.y).is_corner = true;
		at(r.x, r.y+r.h-1).is_corner = true;
		at(r.x+r.w-1, r.y).is_corner = true;
		at(r.x+r.w-1, r.y+r.h-1).is_corner = true;
	}
	bool verify(const Region& r, bool check_init = true){
		if (r.x + r.w > w) return false;
		if (r.y + r.h > h) return false;
		Sqr* sqrs = s + r.y * w;
		if (check_init)
		for (int y = r.y; y < r.y + r.h; y++){
			for (int x = r.x; x < r.x + r.w; x++){
				if ( sqrs[x].passable != 0 || sqrs[x].perimeter != 0 ||
						sqrs[x].roomID != 0)
					return false;
			}
			sqrs += w;
		}
		return true;
	}
	void set_region( const Region& r, const Sqr& val){
		Sqr* sqrs = s + r.y * w;
		for (int y = r.y; y < r.y + r.h; y++){
			for (int x = r.x; x < r.x + r.w; x++){
				memcpy(sqrs+x, &val, sizeof(Sqr));
			}
			sqrs += w;
		}
	}
	int region_groupID( const Region& r, Pos& p){
		Sqr* sqrs = s + r.y * w;
		for (int y = r.y; y < r.y + r.h; y++){
			for (int x = r.x; x < r.x + r.w; x++){
				if (sqrs->passable){
					p.x = x, p.y = y;
					return sqrs->groupID;
				}
			}
			sqrs += w;
		}
	}
	void set_region_with_perimeter(const Region& r, const Sqr& val,
			int padding ) {
		Sqr* sqrs = s + r.y * w;
		Sqr val2 = {0,1,0, 0, val.roomID };

		//Assumes at least padding*2xpadding*2 area
		int y = r.y;
		for (; y < r.y + padding; y++) {
			for (int x = r.x; x < r.x + r.w; x++) {
				memcpy(sqrs + x, &val2, sizeof(Sqr));
			}
			sqrs += w;
		}
		for (; y < r.y + r.h - padding; y++) {
			int x = r.x;
			for (; x < r.x + padding; x++)
				memcpy(sqrs + x, &val2, sizeof(Sqr));
			for (; x < r.x + r.w - padding; x++) {
				memcpy(sqrs + x, &val, sizeof(Sqr));
			}
			for (; x < r.x + r.w; x++) {
				memcpy(sqrs + x, &val2, sizeof(Sqr));
			}
			sqrs += w;
		}
		for (; y < r.y + r.h; y++) {
			for (int x = r.x; x < r.x + r.w; x++) {
				memcpy(sqrs + x, &val2, sizeof(Sqr));
			}
			sqrs += w;
		}
		Region rr = {r.x + padding, r.y + padding,
				r.w - padding*2, r.h - padding*2};
		set_region_corners(rr);
	}
};
//return false on failure
static bool generate_room(int room_id, Squares& s, const Region& r, int padding ){
	if (!s.verify(r)) return false;
	Sqr val = {true, false, false, 0, room_id};
	s.set_region_with_perimeter(r, val, padding);
	return true;
}

inline int rand(MTwist& mt, int max){
	return mt.genrand_int32() % max;
}
inline int rand(MTwist& mt, int min, int max){
	return mt.genrand_int32() % (max-min) + min;
}
struct Room {
	Region room_region;
	int groupID;
};

//Adds random terrain features to the room
void modify_room_randomly(MTwist& mt, Room& r){

}

static void random_entrance(MTwist& mt, Region& r, Pos& p, bool& axis, bool& more, int len=2){
	int ind;
	axis = rand(mt, 2), more = rand(mt,2);
	if (axis){
		int rmx = r.x + r.w-len;
		ind = rand(mt, r.x+1, rmx);
		p.y =  more ? r.y + r.h : r.y - 1;
		p.x = ind;
	} else {
		int rmy = r.y + r.h-len;
		ind = rand(mt, r.y+1, rmy);
		p.x =  more ? r.x + r.w : r.x - 1;
		p.y = ind;
	}
}

#include "tunnelgen.h"

void ensure_room_connectivity(Squares& s, vector<Room>& rooms);

void generate_random_level(RoomgenSettings& rs){
	Sqr* sqrs = rs.sqrs;
	int w = rs.w, h = rs.h, room_seed = rs.seed, padding = rs.padding,
		min_room_size = rs.min_room_size, max_room_size = rs.max_room_size;
	int target_rooms = rs.room_number;
	int minr = min_room_size + padding*2, maxr = max_room_size + padding*2;
	if (room_seed == -1) {
		time_t t;
		time(&t);
		room_seed = (int) t;
	}
	MTwist mt(room_seed);
	Squares s = {sqrs, w, h};
	vector<Room> room_list;

	memset(sqrs, 0, w*h*sizeof(Sqr));
	int rx, ry, rw, rh;
	for (int id = 1; id <= target_rooms; id++){
		for (;;){
			rx = rand(mt,w-1-padding)|1, ry = rand(mt,h-1-padding)|1;
			rw = rand(mt, minr,maxr), rh = rand(mt, minr, maxr);
			Region r = {rx, ry, rw, rh};
			Room room = {{rx + padding, ry + padding, rw - padding*2, rh - padding*2},0};

			if (generate_room(id, s,r, padding)){
				printf("Generated room x=%d,y=%d,w=%d,h=%d\n",rx,ry,rw,rh);
				room_list.push_back(room);
				break;
			}

		}
	}

	Pos p;
	bool axis, more;
	for (int i = 0; i < room_list.size(); i++) {
		bool havepath = false;
		TunnelGen<1,20> tg1width(s, mt, i+1);
		TunnelGen<2,20> tg(s, mt, i+1);
		int path_len = 5;
		int paths = rand(mt,1,rs.max_tunnels);
		int attempts = 0;
		//if (rand(mt,8) == 0) paths += 2;
		while (paths > 0) {
			bool small = (havepath && rand(mt,2) == 0 );

			random_entrance(mt, room_list[i].room_region, p, axis, more);
			//if (s.at(p).passable)
			//	continue;
			if (axis) {
				if ( (small && tg1width.generate_y(p, more, path_len)) ||
					 (!small &&	tg.generate_y(p, more, path_len))){
					tg.accept_tunnel_entry = true;
					tg1width.accept_tunnel_entry = true;
					paths--;
					havepath = true;
					path_len = 5;
				}
			} else {
				if ( (small && tg1width.generate_x(p, more, path_len)) ||
					 (!small &&	tg.generate_x(p, more, path_len))){
					tg.accept_tunnel_entry = true;
					tg1width.accept_tunnel_entry = true;
					paths--;
					havepath = true;
					path_len = 5;
				}
			}
			attempts++;
			if (attempts >= 2){
				path_len += 10;
			}
			if (attempts > 15) break;
		}
		//tg.end_room
	}
	ensure_room_connectivity(s, room_list);

	/*Room erosion/dilation*/
	int erode_chance = 5;
	int dilate_chance = 5;
	int remove_corners_chance = 20;
	for (int i = 0; i < room_list.size(); i++){
		Region r = room_list[i].room_region;
		bool erode = rand(mt, 100) <= erode_chance;
		bool dilate = rand(mt, 100) <= dilate_chance;
		bool remove_corners = rand(mt, 100) <= remove_corners_chance;
		for (int y = r.y-1; y < r.y+r.h+1; y++)
			for (int x = r.x-1; x < r.x+r.w+1; x++){
				Sqr& s = sqrs[y*w+x];
				if (s.is_corner && remove_corners) s.passable = false;
			}
	}

}



void floodfill(Squares& s, int y, int x, unsigned short groupID)
{
	int i;
	int j;
	for (i=-1;i<=1;i++){
		for (j=-1;j<=1;j++){
			Sqr& sqr = s.at(i+x,j+y);
			if (i+x < s.w && i+x >= 0 &&
					j+y < s.h && j+y >= 0 &&
					sqr.passable && sqr.groupID != groupID)
			{
				sqr.groupID = groupID;
				floodfill(s, j+y, i+x, groupID);
			}
		}
	}
}

void ensure_room_connectivity(Squares& s, vector<Room>& rooms){
	Pos p;
	int next_id = 2;
	for (int i = 0; i < rooms.size(); i++){
		int id = s.region_groupID(rooms[i].room_region, p);
		if (id == 0){
			floodfill(s, p.y, p.x, next_id);
			rooms[i].groupID = next_id;
			next_id++;
		}
		else
			rooms[i].groupID = id;
	}
	if (next_id == 2)
		return;//Already connected
/*
	bool fixed = false;
	while (!fixed) {
		fixed = true;

		for (int i = 0; i < rooms.size(); i++){

		}
	}*/
	return;
}



void random_location(RoomgenSettings& rs, int& x, int& y){
	if (rs.seed == -1) {
		time_t t;
		time(&t);
		rs.seed = (int) t;
	}
	MTwist mt(rs.seed);
	int ind;
	do {
		x = rand(mt,1,rs.w);
		y = rand(mt,1,rs.h);
		ind = y*rs.w+x;
	} while (rs.sqrs[ind].roomID == 0 || !rs.sqrs[ind].passable || rs.sqrs[ind].is_corner);
}


