/*
 * fov.cpp
 *
 *  Created on: 2011-11-05
 *      Author: 100397561
 */

#include "fov.h"
#include <cmath>
#include <cstdlib>
#include "../gamestate/GameState.h"
#include "../gamestate/GameTiles.h"
#include "../display/tile_data.h"

using namespace std;

/*fov::fov(GameState *gs, int radius, int ptx, int pty, int sub_squares) :
 gs(gs), radius(radius), ptx(ptx), pty(pty), sub_squares(sub_squares),
 {

 }
 */
fov::fov(int radius) :
		gs(NULL), radius(radius), m(radius, radius, radius, radius) {
	radsub = radius;
	int dim = radsub * 2 + 1;
	sight_mask = new char[dim * dim];
}
void fov::calculate(GameState* gs, int ptx, int pty) {
	this->gs = gs;
	this->ptx = ptx, this->pty = pty;
	for (int y = -radsub; y <= radsub; y++) {
		for (int x = -radsub; x <= radsub; x++) {
			if (x * x + y * y < radsub * radsub)
				m.set(x, y);
			else
				m.clear(x, y);
		}
	}

	int dim = radsub * 2 + 1;
	memset(sight_mask, 0, dim * dim);
	permissive::fov(0, 0, m, *this);
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

	return tiles.is_solid(px, py);
}

void fov::visit(short destX, short destY) {
	int dim = radsub * 2 + 1;
	int dx = radsub + destX, dy = radsub + destY;
	sight_mask[dy * dim + dx] = 1;
}

bool fov::within_fov(int grid_x, int grid_y) {
	if (grid_x < ptx - radsub || grid_x > ptx + radsub || grid_y < pty - radsub
			|| grid_y > pty + radsub)
		return false;

	int dim = radsub * 2 + 1;
	int dx = grid_x - ptx + radsub;
	int dy = grid_y - pty + radsub;
	return sight_mask[dy * dim + dx];
}

void fov::matches(int sqr_x, int sqr_y, char* sub_sqrs) {
	memset(sub_sqrs, 0, 1);
	int sx = sqr_x, ex = sx + 1;
	int sy = sqr_y, ey = sy + 1;
	int dim = radsub * 2 + 1;
	sx = max(ptx - radsub, sx), ex = min(ex, ptx + radsub + 1);
	sy = max(pty - radsub, sy), ey = min(ey, pty + radsub + 1);
	for (int y = sy; y < ey; y++) {
		for (int x = sx; x < ex; x++) {
			int sub_ind = (y - sy) + (x - sx);
			int dx = x - ptx + radsub, dy = y - pty + radsub;
			sub_sqrs[sub_ind] = sight_mask[dim * dy + dx];
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

	int dim = radsub * 2 + 1;
	memcpy(ret->sight_mask, this->sight_mask, dim * dim);

	ret->gs = this->gs;
	ret->ptx = this->ptx, ret->pty = this->pty;

	permissiveMaskT* mask = ret->m.getMask();
	int size = alloc_mask_size(mask->width, mask->height);
	memcpy(ret->m.getMask()->mask, this->m.getMask(), size * sizeof(int));

	return ret;
}
