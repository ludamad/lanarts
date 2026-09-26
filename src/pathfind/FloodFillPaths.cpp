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

#include "FloodFillPaths.h"

using namespace std;

// Step costs: 100 straight, 140 diagonal. Every distance is then a multiple of 20, and a step adds
// at most 7 of those units, so 8 buckets (a circular bucket queue) hold every pending distance.
static const int DISTANCE_UNIT = 20, N_BUCKETS = 8;

// Shortest paths from (sx, sy) over the open tiles of the region, as a direction per tile back
// towards the source. Tiles are final when popped, so distances are true shortest-path lengths.
static void floodfill(Grid<FloodFillNode>& path, Size size, int sx, int sy,
		std::vector<int> (&buckets)[N_BUCKETS], std::vector<char>& finished) {
	int alloc_w = path.size().w;
	finished.assign(size.area(), 0);
	for (auto& bucket : buckets) {
		bucket.clear();
	}

	path[Pos(sx, sy)] = FloodFillNode(false, false, 0, 0, 0);
	buckets[0].push_back(sy * size.w + sx);
	int pending = 1;
	for (int unit = 0; pending > 0; unit++) {
		std::vector<int>& bucket = buckets[unit % N_BUCKETS];
		// Pushes during this loop go to later buckets (every step costs at least 5 units)
		for (size_t i = 0; i < bucket.size(); i++) {
			pending--;
			int index = bucket[i];
			int x = index % size.w, y = index / size.w;
			FloodFillNode& node = path.raw_get(y * alloc_w + x);
			if (finished[index] || node.distance != unit * DISTANCE_UNIT) {
				continue; // reached again by a shorter way since it was queued
			}
			finished[index] = 1;
			for (int dy = -1; dy <= +1; dy++) {
				int ny = y + dy;
				if (ny < 0 || ny >= size.h) {
					continue;
				}
				for (int dx = -1; dx <= +1; dx++) {
					int nx = x + dx;
					if ((dx == 0 && dy == 0) || nx < 0 || nx >= size.w || finished[ny * size.w + nx]) {
						continue;
					}
					FloodFillNode& next = path.raw_get(ny * alloc_w + nx);
					if (next.solid) {
						continue;
					}
					bool diagonal = (dx != 0 && dy != 0);
					// No cutting corners
					if (diagonal && (path.raw_get(y * alloc_w + nx).solid || path.raw_get(ny * alloc_w + x).solid)) {
						continue;
					}
					int distance = node.distance + (diagonal ? 140 : 100);
					if (next.open || distance < next.distance) {
						next.open = false;
						next.dx = -dx, next.dy = -dy;
						next.distance = distance;
						buckets[(distance / DISTANCE_UNIT) % N_BUCKETS].push_back(ny * size.w + nx);
						pending++;
					}
				}
			}
		}
		bucket.clear();
	}
}

FloodFillPaths::FloodFillPaths(const BoolGridRef& solidity) {
	initialize(solidity);
}

void FloodFillPaths::initialize(const BoolGridRef& solidity) {
	_solidity = solidity;
	_fill_valid = false;
}

FloodFillPaths::~FloodFillPaths() {
}

void FloodFillPaths::fill_paths_tile_region(const Pos& tile_xy,
		const BBox& area, bool clear_previous) {
	_topleft_xy = area.left_top();
	_size = area.size();

	if (_path.empty() || _size.w > _path.width() || _size.h > _path.height()) {
		int alloc_w = max(_path.width(), power_of_two_round(_size.w));
		int alloc_h = max(_path.height(), power_of_two_round(_size.h));
		_path.resize(Size(alloc_w, alloc_h));
		memset(_path.begin(), 0, alloc_w * alloc_h * sizeof(FloodFillNode));
	}

	// The same source and window over the same walls give the same fill as last time: keep it.
	// (Players and allies refill every step, mostly without having changed tile.)
	if (clear_previous && _fill_valid && tile_xy == _filled_tile && area == _filled_area) {
		bool unchanged = true;
		for (int y = 0; y < _size.h && unchanged; y++) {
			for (int x = 0; x < _size.w; x++) {
				if (get(x, y)->solid != (*_solidity)[Pos(x + area.x1, y + area.y1)]) {
					unchanged = false;
					break;
				}
			}
		}
		if (unchanged) {
			return;
		}
	}
	_fill_valid = false;

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

	if (_size.area() <= 0 || tile_xy.x < 0 || tile_xy.y < 0 || tile_xy.x >= _size.w || tile_xy.y >= _size.h) {
		return;
	}
	floodfill(_path, _size, tile_xy.x, tile_xy.y, _buckets, _finished);
	if (clear_previous) {
		_fill_valid = true, _filled_tile = tile_xy, _filled_area = area;
	}
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
			// Cells clamped into the window from a box outside it lie beyond the box (a cell just
			// touching its edge still counts, with weight 1)
			if (ex < sx || ey < sy) {
				continue;
			}
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
	float mag = sqrt(float(acc_x) * float(acc_x) + float(acc_y) * float(acc_y));
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
	_fill_valid = false; // edits the fill in place
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
	_fill_valid = false; // edits the fill in place
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
