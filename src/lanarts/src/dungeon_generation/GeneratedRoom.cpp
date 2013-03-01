/*
 * GeneratedRoom.cpp:
 * 	Utility class for room generation functions, stores generated room state & where the rooms are located
 */

#include <lcommon/math_util.h>
#include "GeneratedRoom.h"

Sqr& GeneratedRoom::at(const Pos & p) {
	return at(p.x, p.y);
}

Sqr& GeneratedRoom::at(int x, int y) {
	LANARTS_ASSERT(x >= 0 && x < size.w && y >= 0 && y < size.h);
	return s[y * size.w + x];
}

void GeneratedRoom::set_region_corners(const Region & r) {
	at(r.x, r.y).is_corner = true;
	at(r.x, r.y + r.h - 1).is_corner = true;
	at(r.x + r.w - 1, r.y).is_corner = true;
	at(r.x + r.w - 1, r.y + r.h - 1).is_corner = true;
}

Pos GeneratedRoom::get_world_coordinate(const Pos& p) const {
	return centered_multiple(p + world_offset, TILE_SIZE);
}

bool GeneratedRoom::verify(const Region & r, bool check_init) {
	if (r.x + r.w > size.w)
		return false;

	if (r.y + r.h > size.h)
		return false;

	Sqr *sqrs = s + r.y * size.w;
	if (check_init)
		for (int y = r.y; y < r.y + r.h; y++) {
			for (int x = r.x; x < r.x + r.w; x++) {
				if (sqrs[x].passable != 0 || sqrs[x].perimeter != 0
						|| sqrs[x].roomID != 0)
					return false;

			}
			sqrs += size.w;
		}

	return true;
}

void GeneratedRoom::set_region(const Region & r, const Sqr & val) {
	Sqr *sqrs = s + r.y * size.w;
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			memcpy(sqrs + x, &val, sizeof(Sqr));
		}
		sqrs += size.w;
	}

}

int GeneratedRoom::region_groupID(const Region & r, Pos & p) {
	Sqr *sqrs = s + r.y * size.w;
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			if (sqrs->passable) {
				p.x = x, p.y = y;
				return sqrs->groupID;
			}
		}

		sqrs += size.w;
	}
	return 0;
}

void GeneratedRoom::set_circle_with_perimeter(const Region & r,
		const Sqr & val, int padding) {
	Sqr *sqrs = s + r.y * size.w;
	Sqr val2 = Sqr(0, 1, 0, UNSET, 0, val.roomID);
	float cx = r.x + float(r.w) / 2, cy = r.y + float(r.h) / 2;
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			float dx = (x + .5 - cx) / (r.w / 2.0f - 1), dy = (y + .5 - cy)
					/ (r.h / 2.0f - 1);
			if ((dx * dx) + (dy * dy) < 1.0)
				memcpy(sqrs + x, &val, sizeof(Sqr));
//			else
//				memcpy(sqrs + x, &val2, sizeof(Sqr));
		}
		sqrs += size.w;
	}
}

void GeneratedRoom::set_region_with_perimeter(const Region & r,
		const Sqr & val, int padding) {
	Sqr *sqrs = s + r.y * size.w;
	Sqr val2 = Sqr(0, 1, 0, UNSET, 0, val.roomID);
	int y = r.y;
	for (; y < r.y + padding; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			memcpy(sqrs + x, &val2, sizeof(Sqr));
		}
		sqrs += size.w;
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
		sqrs += size.w;
	}

	for (; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			memcpy(sqrs + x, &val2, sizeof(Sqr));
		}
		sqrs += size.w;
	}

	Region rr = Region(r.x + padding, r.y + padding, r.w - padding * 2,
			r.h - padding * 2);
	set_region_corners(rr);
}

Pos generate_location(MTwist& mt, GeneratedRoom& level) {
	return generate_location_in_region(mt, level,
			Region(0, 0, level.width(), level.height()));
}

Pos generate_location_in_region(MTwist& mt, GeneratedRoom& level,
		const Region& r) {
	Pos p;
	Sqr* s;
	int tries = 0;
	do {
		p.x = mt.rand(r.x, r.x + r.w);
		p.y = mt.rand(r.y, r.y + r.h);
		s = &level.at(p.x, p.y);
		tries++;
	} while (tries < 100 && (!s->passable || s->is_corner || s->has_instance));

	if (tries >= 100 && r.w < level.width() && r.h < level.height())
		return generate_location(mt, level);

	return p;
}

Pos generate_location_byroom(MTwist& mt, GeneratedRoom& level) {
	int roomn = mt.rand(level.rooms().size());
	RoomRegion& room = level.rooms()[roomn];
	return generate_location_in_region(mt, level, room.region);
}
