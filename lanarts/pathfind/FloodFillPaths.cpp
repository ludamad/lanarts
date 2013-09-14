/*
 * FloodFillPaths.cpp:
 *  Utilities for flood-fill based pathfinding
 */

#include <cmath>

#include <ldraw/colour_constants.h>
#include <ldraw/DrawOptions.h>

#include "draw/TileEntry.h"

#include "gamestate/GameState.h"
#include "gamestate/GameTiles.h"

#include <lcommon/math_util.h>

#include "gheap.h"
#include "FloodFillPaths.h"

using namespace std;

static void floodfill(Grid<FloodFillNode>& path, Size size, int sx, int sy) {
	FloodFillCoord* heap = new FloodFillCoord[size.area()];
	FloodFillCoord* heap_end = heap + 1;

	Size alloc_size = path.size();

	heap[0] = FloodFillCoord(sx, sy, 0); //Start coordinate
	path[Pos(sx, sy)] = FloodFillNode(false, false, 0, 0, 0);
	while (heap != heap_end) {
		FloodFillCoord curr = *heap;
		FloodFillCoord next;
		gheap<>::pop_heap(heap, heap_end--);
		for (int dy = -1; dy <= +1; dy++) {
			for (int dx = -1; dx <= +1; dx++) {
				int nx = curr.x + dx, ny = curr.y + dy;
				//LANARTS_ASSERT(
				//		curr.x >= 0 && curr.x < w && curr.y >= 0 && curr.y < h);
				if (nx < 0 || nx >= size.w || ny < 0 || ny >= size.h)
					continue;
				int coord = ny * alloc_size.w + nx;
				bool is_diag = (abs(dx) == abs(dy));
				int dist = curr.distance + (is_diag ? 140 : 100);
				FloodFillNode* p = &path.raw_get(coord);
				if (p->open && !p->solid) {
					bool cant_cross = is_diag
							&& (path[Pos(nx, curr.y)].solid
									|| path[Pos(curr.x, ny)].solid);
					if (!cant_cross) {
						p->open = false;
						p->dx = -dx, p->dy = -dy;
						p->distance = dist;
						*(heap_end++) = FloodFillCoord(nx, ny, dist);
						gheap<>::push_heap(heap, heap_end);
					}
				}
			}

		}
	}

	delete[] heap;
}

FloodFillPaths::FloodFillPaths(const BoolGridRef& solidity) {
	initialize(solidity);
}

void FloodFillPaths::initialize(const BoolGridRef& solidity) {
	_solidity = solidity;
}

FloodFillPaths::~FloodFillPaths() {
}

void FloodFillPaths::fill_paths_tile_region(const Pos& tile_xy,
		const BBox& area, bool clear_previous) {
	_topleft_xy = area.left_top();
	_size = area.size();

	if (_path.empty() || _size.w < _path.width() || _size.h < _path.height()) {
		int alloc_w = max(_path.width(), power_of_two_round(_size.w));
		int alloc_h = max(_path.height(), power_of_two_round(_size.h));
		_path.resize(Size(alloc_w, alloc_h));
		memset(_path.begin(), 0, alloc_w * alloc_h * sizeof(FloodFillNode));
	}

	if (clear_previous) {
		for (int y = 0; y < _size.h; y++) {
			for (int x = 0; x < _size.w; x++) {
				FloodFillNode* node = get(x, y);
				node->solid = (*_solidity)[Pos(x + area.x1, y + area.y1)];
				node->open = true;
				node->dx = 0;
				node->dy = 0;
				node->distance = 0;
			}
		}
	}

	floodfill(_path, _size, tile_xy.x, tile_xy.y);
}

void FloodFillPaths::fill_paths_in_radius(const Pos& source_xy, int radius) {
	perf_timer_begin(FUNCNAME);

	//Use a temporary 'GameView' object to make use of its helper methods
	GameView view(0, 0, radius * 2, radius * 2, _solidity->width() * TILE_SIZE,
			_solidity->height() * TILE_SIZE);
	view.sharp_center_on(source_xy);

	BBox tiles_covered = view.tile_region_covered();

	Pos tile_xy = source_xy.divided(TILE_SIZE) - tiles_covered.left_top();
	fill_paths_tile_region(tile_xy, tiles_covered);

	perf_timer_end(FUNCNAME);
}

bool FloodFillPaths::is_solid_or_out_of_bounds(int x, int y) {
	if (x < 0 || x >= width() || y < 0 || y >= height()) {
		return true;
	}

	return get(x, y)->solid;
}

bool FloodFillPaths::can_head(const BBox& bbox, int speed, int dx, int dy) {
	bool is_diag = (abs(dx) == abs(dy));

	int xx, yy;
	for (int y = bbox.y1; y <= bbox.y2 + TILE_SIZE; y += TILE_SIZE) {
		for (int x = bbox.x1; x <= bbox.x2 + TILE_SIZE; x += TILE_SIZE) {
			xx = squish(x, bbox.x1, bbox.x2 + 1);
			yy = squish(y, bbox.y1, bbox.y2 + 1);

			int gx = (xx + dx * speed) / TILE_SIZE - _topleft_xy.x;
			int gy = (yy + dy * speed) / TILE_SIZE - _topleft_xy.y;
			if (is_solid_or_out_of_bounds(gx, gy)) {
				return false;
			}
			if (is_diag) {
				if (is_solid_or_out_of_bounds(xx / TILE_SIZE - _topleft_xy.x,
						(yy + dy * speed) / TILE_SIZE - _topleft_xy.y)) {
					return false;
				}
				if (is_solid_or_out_of_bounds(
						(xx + dx * speed) / TILE_SIZE - _topleft_xy.x,
						yy / TILE_SIZE - _topleft_xy.y)) {
					return false;
				}
			}
		}
	}
	return true;
}

//Away from object
PosF FloodFillPaths::random_further_direction(MTwist& mt, int x, int y, int w,
		int h, float speed) {

	if (_path.empty()) {
		return PosF();
	}

	int mx = x + w, my = y + h;

	//Set up coordinate min and max
	int mingrid_x = x / TILE_SIZE, mingrid_y = y / TILE_SIZE;
	int maxgrid_x = mx / TILE_SIZE, maxgrid_y = my / TILE_SIZE;

	//Make sure coordinates do not go out of bounds
	int minx = squish(mingrid_x, _topleft_xy.x, _topleft_xy.x + width());
	int miny = squish(mingrid_y, _topleft_xy.y, _topleft_xy.y + height());
	int maxx = squish(maxgrid_x, _topleft_xy.x, _topleft_xy.x + width());
	int maxy = squish(maxgrid_y, _topleft_xy.y, _topleft_xy.y + height());

	//Set up accumulators for x and y (later normalized)
	int acc_x = 0, acc_y = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int px = xx - _topleft_xy.x, py = yy - _topleft_xy.y;
			FloodFillNode* p = get(px, py);
			if (!p->solid) {
				point_to_random_further(mt, px, py);
			}
		}
	}

	return interpolated_direction(BBox(x, y, x + w, y + h), speed);
}

PosF FloodFillPaths::interpolated_direction(const BBox& bbox, float speed,
		bool lenient) {

	if (_path.empty()) {
		return PosF();
	}

	int ispeed = (int) ceil(speed);
	int area = bbox.size().area();

	//Set up coordinate min and max
	int mingrid_x = bbox.x1 / TILE_SIZE, mingrid_y = bbox.y1 / TILE_SIZE;
	int maxgrid_x = bbox.x2 / TILE_SIZE, maxgrid_y = bbox.y2 / TILE_SIZE;
	//Make sure coordinates do not go out of bounds
	int minx = squish(mingrid_x, _topleft_xy.x, _topleft_xy.x + width());
	int miny = squish(mingrid_y, _topleft_xy.y, _topleft_xy.y + height());
	int maxx = squish(maxgrid_x, _topleft_xy.x, _topleft_xy.x + width());
	int maxy = squish(maxgrid_y, _topleft_xy.y, _topleft_xy.y + height());
	//Set up accumulators for x and y (later normalized)
	int acc_x = 0, acc_y = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int sx = max(xx * TILE_SIZE, bbox.x1), sy = max(yy * TILE_SIZE,
					bbox.y1);
			int ex = min((xx + 1) * TILE_SIZE, bbox.x2), ey = min(
					(yy + 1) * TILE_SIZE, bbox.y2);
			int px = xx - _topleft_xy.x, py = yy - _topleft_xy.y;
			FloodFillNode* p = get(px, py);
			if (!p->solid) {
				int sub_area = (ex - sx) * (ey - sy) + 1;
				/*Make sure all interpolated directions are possible*/
				if (lenient || can_head(bbox, ispeed, p->dx, p->dy)) {
					acc_x += p->dx * sub_area;
					acc_y += p->dy * sub_area;
				}
			}
		}
	}
	float mag = sqrt(float(acc_x * acc_x + acc_y * acc_y));
	if (mag == 0) {
		if (!lenient) {
			return interpolated_direction(bbox, speed, true);
		} else {
			return PosF();
		}
	} else {
		float vx = speed * float(acc_x) / mag;
		float vy = speed * float(acc_y) / mag;
		return PosF(vx, vy);
	}
}

const int HUGE_DISTANCE = 1000000;

void FloodFillPaths::point_to_local_min(int sx, int sy) {
	FloodFillNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, width()), miny = squish(sy - 1, 0, height());
	int maxx = squish(sx + 1, 0, width()), maxy = squish(sy + 1, 0, height());

	int dx = 0, dy = 0;
	int min_distance = HUGE_DISTANCE;

	if (!fixed_node->open) {
		for (int yy = miny; yy <= maxy; yy++) {
			for (int xx = minx; xx <= maxx; xx++) {
				if (sx == xx && sy == yy)
					continue;
				FloodFillNode* p = get(xx, yy);
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

void FloodFillPaths::point_to_random_further(MTwist& mt, int sx, int sy) {
	FloodFillNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, _size.w), miny = squish(sy - 1, 0, _size.h);
	int maxx = squish(sx + 1, 0, _size.w), maxy = squish(sy + 1, 0, _size.h);

	int dx = 0, dy = 0;
	int compare_distance = fixed_node->distance;
	bool set = false;

	if (!fixed_node->open) {
		for (int yy = miny; yy <= maxy; yy++) {
			for (int xx = minx; xx <= maxx; xx++) {
				if (sx == xx && sy == yy)
					continue;
				FloodFillNode* p = get(xx, yy);
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

void FloodFillPaths::debug_draw(GameState* gs) {
	GameView& view = gs->view();

	for (int y = 0; y < _size.h; y++) {
		for (int x = 0; x < _size.w; x++) {
			FloodFillNode* node = get(x, y);
			if (false && !node->solid)
				gs->font().drawf(COL_WHITE,
						Pos((x + _topleft_xy.x) * TILE_SIZE - view.x,
								(y + _topleft_xy.y) * TILE_SIZE - view.y), "%d,%d",
						node->dx, node->dy);
			if (!node->solid) {
				gs->font().drawf(COL_WHITE,
						Pos((x + _topleft_xy.x) * TILE_SIZE - view.x,
								(y + _topleft_xy.y) * TILE_SIZE - view.y), "%d",
						node->distance);
			}
		}
	}
}
