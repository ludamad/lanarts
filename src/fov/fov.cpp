/*
 * fov.cpp
 *
 *  Created on: 2011-11-05
 *      Author: 100397561
 */

#include "fov.h"
#include <cmath>
#include <cstdlib>
#include "../world/GameState.h"
#include "../world/GameTiles.h"
#include "../data/tile_data.h"


using namespace std;

/*fov::fov(GameState *gs, int radius, int ptx, int pty, int sub_squares) :
			gs(gs), radius(radius), ptx(ptx), pty(pty), sub_squares(sub_squares),
		{

}
*/
fov::fov(int radius, int sub_squares):
		gs(NULL), radius(radius), sub_squares(sub_squares),
		m(radius * sub_squares, radius * sub_squares,
				radius * sub_squares, radius * sub_squares) {
	radsub = radius * sub_squares;
	int dim = radsub * 2 + 1;
	sight_mask = new char[dim * dim];
}
void fov::calculate(GameState* gs, int ptx, int pty){
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
	GameTiles & tiles = gs->tile_grid();
	int px = ptx + destX, py = pty + destY;
	px = (px)/sub_squares; py = (py)/sub_squares;
	bool outof = (px < 0 || py < 0 || px >= tiles.tile_width()
			|| py >= tiles.tile_height());
	if (outof)
		return false;

	return (game_tile_data[tiles.get(px, py)].solid);
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

void fov::matches(int sqr_x, int sqr_y, char *sub_sqrs) {
	memset(sub_sqrs, 0, sub_squares*sub_squares);
	int sx = sqr_x * sub_squares, ex = sx + sub_squares;
	int sy = sqr_y * sub_squares, ey = sy + sub_squares;
	int dim = radsub * 2 + 1;
	sx = max(ptx-radsub,sx), ex = min(ex, ptx+radsub+1);
	sy = max(pty-radsub,sy), ey = min(ey, pty+radsub+1);
	for (int y = sy; y < ey; y++) {
		for (int x = sx; x < ex; x++) {
			int sub_ind = (y-sy)*sub_squares + (x - sx);
			int dx = x - ptx + radsub, dy = y - pty + radsub;
			sub_sqrs[sub_ind] = sight_mask[dim * dy + dx];
		}
	}

}

