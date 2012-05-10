/*
 * GeneratedLevel.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "GeneratedLevel.h"

Sqr & GeneratedLevel::at(const Pos & p) {
	return s[p.y * w + p.x];
}

Sqr & GeneratedLevel::at(int x, int y) {
	return s[y * w + x];
}

void GeneratedLevel::set_region_corners(const Region & r) {
	at(r.x, r.y).is_corner = true;
	at(r.x, r.y + r.h - 1).is_corner = true;
	at(r.x + r.w - 1, r.y).is_corner = true;
	at(r.x + r.w - 1, r.y + r.h - 1).is_corner = true;
}

bool GeneratedLevel::verify(const Region & r, bool check_init) {
	if (r.x + r.w > w)
		return false;

	if (r.y + r.h > h)
		return false;

	Sqr *sqrs = s + r.y * w;
	if (check_init)
		for (int y = r.y; y < r.y + r.h; y++) {
			for (int x = r.x; x < r.x + r.w; x++) {
				if (sqrs[x].passable != 0 || sqrs[x].perimeter != 0
						|| sqrs[x].roomID != 0)
					return false;

			}
			sqrs += w;
		}

	return true;
}

void GeneratedLevel::set_region(const Region & r, const Sqr & val) {
	Sqr *sqrs = s + r.y * w;
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			memcpy(sqrs + x, &val, sizeof(Sqr));
		}
		sqrs += w;
	}

}

int GeneratedLevel::region_groupID(const Region & r, Pos & p) {
	Sqr *sqrs = s + r.y * w;
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			if (sqrs->passable) {
				p.x = x, p.y = y;
				return sqrs->groupID;
			}
		}

		sqrs += w;
	}
	return 0;
}

void GeneratedLevel::set_circle_with_perimeter(const Region & r, const Sqr & val,
		int padding) {
	Sqr *sqrs = s + r.y * w;
	Sqr val2 = Sqr(0, 1, 0, UNSET, 0, val.roomID);
	float cx = r.x + float(r.w)/2, cy = r.y + float(r.h)/2;
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			float dx = (x +.5 - cx)/(r.w/2.0f - 1), dy = (y + .5 - cy)/(r.h/2.0f - 1);
			if ((dx*dx)+(dy*dy) < 1.0)
				memcpy(sqrs + x, &val, sizeof(Sqr));
//			else
//				memcpy(sqrs + x, &val2, sizeof(Sqr));
		}
		sqrs += w;
	}
}

void GeneratedLevel::set_region_with_perimeter(const Region & r, const Sqr & val,
		int padding) {
	Sqr *sqrs = s + r.y * w;
	Sqr val2 = Sqr(0, 1, 0, UNSET, 0, val.roomID);
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

	Region rr = Region( r.x + padding, r.y + padding, r.w - padding * 2, r.h
			- padding * 2 );
	set_region_corners(rr);
}


Pos generate_location(MTwist& mt, GeneratedLevel& level) {
	return generate_location_in_region(mt, level, Region(0,0, level.width(), level.height()));
}

Pos generate_location_in_region(MTwist& mt, GeneratedLevel& level, const Region& r) {
	Pos p;
	Sqr* s;
	int tries = 0;
	do {
		p.x = mt.rand(r.x,r.x+r.w);
		p.y = mt.rand(r.y, r.y+r.h);
		s = &level.at(p.x, p.y);
		tries++;
	} while ( tries < 100 && (!s->passable || s->is_corner || s->has_instance));

	if (tries >= 100)
		return generate_location(mt, level);

	return p;
}

Pos generate_location_byroom(MTwist& mt, GeneratedLevel& level) {
	int roomn = mt.rand(level.rooms().size());
	Room& room = level.rooms()[roomn];
	return generate_location_in_region(mt, level, room.room_region);
}
