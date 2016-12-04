/*
 * fov.cpp:
 *  Represents a field of view
 */

#include <cmath>
#include <cstdlib>

#include "draw/TileEntry.h"

#include "gamestate/GameState.h"
#include "gamestate/GameTiles.h"

#include "fov.h"

using namespace std;

fov::fov(int radius) :
		gs(NULL), radius(radius), m(radius, radius, radius, radius), ptx(0), pty(
				0), sx(0), sy(0) {
	diameter = radius * 2 + 1;
	has_been_calculated = false;
	sight_mask = new char[diameter * diameter];
}

void fov::calculate(GameState* gs, int subradius, int ptx, int pty) {
	perf_timer_begin(FUNCNAME);
        LANARTS_ASSERT(subradius <= radius);

	this->has_been_calculated = true;
	this->gs = gs;
	this->ptx = ptx, this->pty = pty;
	this->sx = ptx - radius, this->sy = pty - radius;

	float subradius_squared = (subradius-.5) * (subradius-.5);

	for (int y = -radius; y <= radius; y++) {
		for (int x = -radius; x <= radius; x++) {

			if (x * x + y * y < subradius_squared) {
				m.set(x, y);
			} else {
				m.clear(x, y);
			}
		}
	}

	memset(sight_mask, 0, diameter * diameter);
	permissive::fov(0, 0, m, *this);

	perf_timer_end(FUNCNAME);
}

fov::~fov() {
	delete[] sight_mask;
}

int fov::isBlocked(short destX, short destY) {
	GameTiles & tiles = gs->tiles();
	int px = ptx + destX, py = pty + destY;
	bool outof = (px < 0 || py < 0 || px >= tiles.tile_width()
			|| py >= tiles.tile_height());
	if (outof)
		return false;

	return !tiles.is_seethrough(Pos(px, py));
}

bool fov::within_fov(const BBox& bbox) {
	if (!has_been_calculated) {
		return false;
	}
	int ex = sx + diameter, ey = sy + diameter;
	if (bbox.x2 < sx || bbox.x1 >= ex || bbox.y2 < sy || bbox.y1 >= ey) {
		return false;
	}
	BBox sbox(std::max(bbox.x1, sx), std::max(bbox.y1, sy),
			std::min(bbox.x2, ex - 1), std::min(bbox.y2, ey - 1));
	char* mask = sight_mask + (sbox.y1 - sy) * diameter;
	for (int y = sbox.y1; y <= sbox.y2; y++) {
		for (int x = sbox.x1; x <= sbox.x2; x++) {
			LANARTS_ASSERT(
					!(x < ptx - radius || x > ptx + radius || y < pty - radius || y > pty + radius));
			if (mask[x - sx]) {
				return true;
			}
		}
		mask += diameter;
	}
	return false;
}

void fov::visit(short destX, short destY) {
	int dx = radius + destX, dy = radius + destY;
	sight_mask[dy * diameter + dx] = 1;
}

bool fov::within_fov(int grid_x, int grid_y) {
	if (grid_x < ptx - radius || grid_x > ptx + radius || grid_y < pty - radius
			|| grid_y > pty + radius)
		return false;

	int dx = grid_x - ptx + radius;
	int dy = grid_y - pty + radius;
	return sight_mask[dy * diameter + dx];
}

void fov::matches(int sqr_x, int sqr_y, char* sub_sqrs) {
	memset(sub_sqrs, 0, 1);
	int sx = sqr_x, ex = sx + 1;
	int sy = sqr_y, ey = sy + 1;

	sx = max(ptx - radius, sx), ex = min(ex, ptx + radius + 1);
	sy = max(pty - radius, sy), ey = min(ey, pty + radius + 1);
	for (int y = sy; y < ey; y++) {
		for (int x = sx; x < ex; x++) {
			int sub_ind = (y - sy) + (x - sx);
			int dx = x - ptx + radius, dy = y - pty + radius;
			sub_sqrs[sub_ind] = sight_mask[diameter * dy + dx];
		}
	}

}

static int alloc_mask_size(int w, int h) {
	static const int BITS_PER_INT = sizeof(int) * 8;

	int cellCount = w * h;
	int intCount = cellCount / BITS_PER_INT;
	if (cellCount % BITS_PER_INT != 0) {
		++intCount;
	}
	return intCount;
}

fov* fov::clone() const {
	fov* ret = new fov(radius);

	memcpy(ret->sight_mask, this->sight_mask, diameter * diameter);

	ret->gs = this->gs;
	ret->ptx = this->ptx, ret->pty = this->pty;

	permissiveMaskT* mask = ret->m.getMask();
	int size = alloc_mask_size(mask->width, mask->height);
	memcpy(ret->m.getMask()->mask, this->m.getMask(), size * sizeof(int));

	return ret;
}
