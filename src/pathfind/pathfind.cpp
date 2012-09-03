/*
 * pathfind.cpp:
 *  Utilities for flood-fill based pathfinding
 */

#include "gheap.h"
#include <cmath>
#include "pathfind.h"
#include "../display/tile_data.h"
#include "../gamestate/GameState.h"
#include "../gamestate/GameTiles.h"
#include "../util/math_util.h"

using namespace std;

void floodfill(PathingNode* path, int w, int h, int sx, int sy, int alloc_w) {
	PathCoord* heap = new PathCoord[w * h];
	PathCoord* heap_end = heap + 1;

	heap[0] = PathCoord(sx, sy, 0); //Start coordinate
	path[sy * alloc_w + sx] = PathingNode(false, false, 0, 0, 0);
	while (heap != heap_end) {
		PathCoord curr = *heap;
		PathCoord next;
		gheap<>::pop_heap(heap, heap_end--);
		for (int dy = -1; dy <= +1; dy++) {
			for (int dx = -1; dx <= +1; dx++) {
				int nx = curr.x + dx, ny = curr.y + dy;
				LANARTS_ASSERT(
						curr.x >= 0 && curr.x < w && curr.y >= 0 && curr.y < h);
				if (nx < 0 || nx >= w || ny < 0 || ny >= h)
					continue;
				int coord = ny * alloc_w + nx;
				bool is_diag = (abs(dx) == abs(dy));
				int dist = curr.distance + (is_diag ? 140 : 100);
				PathingNode* p = &path[coord];
				if (p->open && !p->solid) {
					bool cant_cross = is_diag
							&& (path[curr.y * alloc_w + nx].solid
									|| path[ny * alloc_w + curr.x].solid);
					if (!cant_cross) {
						p->open = false;
						p->dx = -dx, p->dy = -dy;
						p->distance = dist;
						*(heap_end++) = PathCoord(nx, ny, dist);
						gheap<>::push_heap(heap, heap_end);
					}
				}
			}

		}
	}

	delete[] heap;
}

PathInfo::PathInfo() {
	path = NULL;
	w = 0, h = 0;
	alloc_w = 0, alloc_h = 0;
	start_x = 0, start_y = 0;
}
PathInfo::~PathInfo() {
	delete[] path;
}
void PathInfo::calculate_path(GameState* gs, int ox, int oy, int radius) {
	perf_timer_begin(FUNCNAME);
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	GameTiles& tile = gs->tiles();

	//Use a temporary 'GameView' object to make use of its helper methods
	GameView view(0, 0, radius * 2, radius * 2, gs->width(), gs->height());
	view.sharp_center_on(ox, oy);

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	start_x = min_tilex, start_y = min_tiley;

	int ww = max_tilex - min_tilex, hh = max_tiley - min_tiley;
	w = ww, h = hh;
	if (!path || w < alloc_w || h < alloc_h) {
		alloc_w = max(alloc_w, power_of_two(w));
		alloc_h = max(alloc_h, power_of_two(h));
		if (path)
			delete[] path;
		path = new PathingNode[alloc_w * alloc_h];
		memset(path, 0, alloc_w * alloc_h * sizeof(PathingNode));
	}
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			PathingNode* node = get(x, y);
			node->solid = tile.is_solid(x + min_tilex, y + min_tiley);
			node->open = true;
			node->dx = 0;
			node->dy = 0;
			node->marked = false;
			node->distance = 0;
		}
	}
	int tx = ox / TILE_SIZE - min_tilex, ty = oy / TILE_SIZE - min_tiley;
	floodfill(path, w, h, tx, ty, alloc_w);
	path_x = tx, path_y = ty;
	perf_timer_end(FUNCNAME);
}

static bool is_solid_or_out_of_bounds(PathInfo& path, int x, int y) {
	if (x < 0 || x >= path.width() || y < 0 || y >= path.height())
		return true;
	return path.get(x, y)->solid;
}

bool PathInfo::can_head(int sx, int sy, int ex, int ey, int speed, int dx,
		int dy) {
	bool is_diag = (abs(dx) == abs(dy));

	int xx, yy;
	for (int y = sy; y <= ey + TILE_SIZE; y += TILE_SIZE) {
		for (int x = sx; x <= ex + TILE_SIZE; x += TILE_SIZE) {
			xx = squish(x, sx, ex + 1);
			yy = squish(y, sy, ey + 1);
			int gx = (xx + dx * speed) / TILE_SIZE - start_x;
			int gy = (yy + dy * speed) / TILE_SIZE - start_y;
			if (is_solid_or_out_of_bounds(*this, gx, gy)) {
				return false;
			}
			if (is_diag) {
				if (is_solid_or_out_of_bounds(*this, xx / TILE_SIZE - start_x,
						(yy + dy * speed) / TILE_SIZE - start_y)) {
					return false;
				}
				if (is_solid_or_out_of_bounds(*this,
						(xx + dx * speed) / TILE_SIZE - start_x,
						yy / TILE_SIZE - start_y)) {
					return false;
				}
			}
		}
	}
	return true;
}

//Away from object
void PathInfo::random_further_direction(MTwist& mt, int x, int y, int w, int h,
		float speed, float& vx, float& vy) {
	if (!path) {
		vx = 0, vy = 0;
		return;
	}
	int mx = x + w, my = y + h;
	//Set up coordinate min and max
	int mingrid_x = x / TILE_SIZE, mingrid_y = y / TILE_SIZE;
	int maxgrid_x = mx / TILE_SIZE, maxgrid_y = my / TILE_SIZE;
	//Make sure coordinates do not go out of bounds
	int minx = squish(mingrid_x, start_x, start_x + width());
	int miny = squish(mingrid_y, start_y, start_y + height());
	int maxx = squish(maxgrid_x, start_x, start_x + width());
	int maxy = squish(maxgrid_y, start_y, start_y + height());
	//Set up accumulators for x and y (later normalized)
	int acc_x = 0, acc_y = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int px = xx - start_x, py = yy - start_y;
			PathingNode* p = get(px, py);
			if (!p->solid) {
				point_to_random_further(mt, px, py);
			}
		}
	}
	interpolated_direction(x, y, w, h, speed, vx, vy);
}

void PathInfo::interpolated_direction(int x, int y, int w, int h, float speed,
		float& vx, float& vy) {
	if (!path) {
		vx = 0, vy = 0;
		return;
	}
	int ispeed = (int)ceil(speed);
	int area = w * h;
	int mx = x + w, my = y + h;
	//Set up coordinate min and max
	int mingrid_x = x / TILE_SIZE, mingrid_y = y / TILE_SIZE;
	int maxgrid_x = mx / TILE_SIZE, maxgrid_y = my / TILE_SIZE;
	//Make sure coordinates do not go out of bounds
	int minx = squish(mingrid_x, start_x, start_x + width());
	int miny = squish(mingrid_y, start_y, start_y + height());
	int maxx = squish(maxgrid_x, start_x, start_x + width());
	int maxy = squish(maxgrid_y, start_y, start_y + height());
	//Set up accumulators for x and y (later normalized)
	int acc_x = 0, acc_y = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int sx = max(xx * TILE_SIZE, x), sy = max(yy * TILE_SIZE, y);
			int ex = min((xx + 1) * TILE_SIZE, mx), ey = min(
					(yy + 1) * TILE_SIZE, my);
			int px = xx - start_x, py = yy - start_y;
			PathingNode* p = get(px, py);
			if (!p->solid) {
				int sub_area = (ex - sx) * (ey - sy) + 1;
				/*Make sure all interpolated directions are possible*/
				if (can_head(x, y, mx, my, ispeed, p->dx, p->dy)) {
					acc_x += p->dx * sub_area;
					acc_y += p->dy * sub_area;
				}
			}
		}
	}
	float mag = sqrt(float(acc_x * acc_x + acc_y * acc_y));
	if (mag == 0) {
		vx = 0, vy = 0;
	} else {
		vx = speed * float(acc_x) / mag;
		vy = speed * float(acc_y) / mag;
	}
}

const int HUGE_DISTANCE = 1000000;

void PathInfo::point_to_local_min(int sx, int sy) {
	PathingNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, width()), miny = squish(sy - 1, 0, height());
	int maxx = squish(sx + 1, 0, width()), maxy = squish(sy + 1, 0, height());

	int dx = 0, dy = 0;
	int min_distance = HUGE_DISTANCE;

	if (!fixed_node->marked) {
		for (int yy = miny; yy <= maxy; yy++) {
			for (int xx = minx; xx <= maxx; xx++) {
				if (sx == xx && sy == yy)
					continue;
				PathingNode* p = get(xx, yy);
				if (p->solid)
					continue;
				int dist = p->distance
						+ (abs(xx - sx) == abs(yy - sy) ? 140 : 100);
				if (dist < min_distance) {
					dx = xx - sx, dy = yy - sy;
					min_distance = dist;
				}
			}
		}
	}
	fixed_node->dx = dx;
	fixed_node->dy = dy;
//	if (dx == 0 && dy == 0) {
//		fixed_node->distance = HUGE_DISTANCE;
//	} else {
//		fixed_node->distance = min_distance + (abs(dx) == abs(dy) ? 140 : 100);
//	}
}

void PathInfo::point_to_random_further(MTwist& mt, int sx, int sy) {
	PathingNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, width()), miny = squish(sy - 1, 0, height());
	int maxx = squish(sx + 1, 0, width()), maxy = squish(sy + 1, 0, height());

	int dx = 0, dy = 0;
	int compare_distance = fixed_node->distance;
	bool set = false;

	if (!fixed_node->marked) {
		for (int yy = miny; yy <= maxy; yy++) {
			for (int xx = minx; xx <= maxx; xx++) {
				if (sx == xx && sy == yy)
					continue;
				PathingNode* p = get(xx, yy);
				if (p->solid)
					continue;
				int dist = p->distance;
				if (dist >= compare_distance && (!set || mt.rand(2))) {
					dx = xx - sx, dy = yy - sy;
					set = true;
				}
			}
		}
	}
	fixed_node->dx = dx;
	fixed_node->dy = dy;
//	if (dx == 0 && dy == 0) {
//		fixed_node->distance = HUGE_DISTANCE;
//	} else {
//		fixed_node->distance = min_distance + (abs(dx) == abs(dy) ? 140 : 100);
//	}
}

void PathInfo::fix_distances(int sx, int sy) {
	PathingNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, width()), miny = squish(sy - 1, 0, height());
	int maxx = squish(sx + 1, 0, width()), maxy = squish(sy + 1, 0, height());

	int min_distance = fixed_node->distance;
	if (fixed_node->marked)
		return;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			if (sx == xx && sy == yy)
				continue;
			PathingNode* p = get(xx, yy);
			if (p->solid)
				continue;
			int dist = p->distance + (abs(xx - sx) == abs(yy - sy) ? 140 : 100);
			if (dist < min_distance) {
				min_distance = dist;
			}
		}
	}
	fixed_node->distance = min_distance;
}
void PathInfo::adjust_for_claims(int x, int y) {
	x = x / TILE_SIZE - start_x, y = y / TILE_SIZE - start_y;
	if (x < 0 || x >= width() || y < 0 || y >= height())
		return;
	int minx = squish(x - 1, 0, width()), miny = squish(y - 1, 0, height());
	int maxx = squish(x + 1, 0, width()), maxy = squish(y + 1, 0, height());
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			this->point_to_local_min(xx, yy);
		}
	}
}
void PathInfo::stake_claim(int x, int y) {
	x = x / TILE_SIZE - start_x, y = y / TILE_SIZE - start_y;
	if (x < 0 || x >= width() || y < 0 || y >= height())
		return;

	PathingNode* start_node = get(x, y);
	x += start_node->dx, y += start_node->dy;
	//Don't claim the player's square
	if (x == path_x && y == path_y) {
		//Backtrack, and stake our current square
		x -= start_node->dx, y -= start_node->dy;
	}
	PathingNode* stake_node = get(x, y);

	//Make distance some arbitrarily large number
	stake_node->distance = HUGE_DISTANCE;
	stake_node->marked = true;
//	stake_node->solid = true;

	int minx = squish(x - 1, 0, width()), miny = squish(y - 1, 0, height());
	int maxx = squish(x + 1, 0, width()), maxy = squish(y + 1, 0, height());
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			this->fix_distances(xx, yy);
		}
	}
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			this->point_to_local_min(xx, yy);
		}
	}

//	stake_node->solid = false;
}

void PathInfo::draw(GameState* gs) {
	GameView& view = gs->view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			PathingNode* node = get(x, y);
			if (false && !node->solid)
				gl_printf(gs->primary_font(), Colour(255, 255, 255),
						(x + start_x) * TILE_SIZE - view.x,
						(y + start_y) * TILE_SIZE - view.y, "%d,%d", node->dx,
						node->dy);
			if (!node->solid)
				gl_printf(gs->primary_font(), Colour(255, 255, 255),
						(x + start_x) * TILE_SIZE - view.x,
						(y + start_y) * TILE_SIZE - view.y, "%d",
						node->distance);
		}
	}
}
