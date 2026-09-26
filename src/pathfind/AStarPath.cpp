/*
 * AStarPath.cpp:
 *  Implements A* pathfinding
 */

#include <algorithm>
#include <ldraw/draw.h>

#include "draw/TileEntry.h"

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"

#include "gamestate/GameTiles.h"
#include "AStarPath.h"


using namespace std;

static inline int heurestic_distance(const Pos& s, const Pos& e) {
	int h_diagonal = min(abs(s.x - e.x), abs(s.y - e.y));
	int h_straight = (abs(s.x - e.x) + abs(s.y - e.y));
	return 114 * h_diagonal + 100 * (h_straight - 2 * h_diagonal);
}

namespace {
	// Heap entry; a node whose g_score has since improved leaves stale entries behind, skipped when
	// popped (cheaper than finding and updating its entry)
	struct AStarEntry {
		int f_score, h_score, g_score, index;
	};
	// std heaps keep the greatest on top: order so that it is the lowest f_score, and among equal
	// f_scores the one nearest the goal
	struct AStarEntryOrder {
		bool operator()(const AStarEntry& a, const AStarEntry& b) const {
			if (a.f_score != b.f_score) {
				return a.f_score > b.f_score;
			}
			if (a.h_score != b.h_score) {
				return a.h_score > b.h_score;
			}
			return a.index > b.index;
		}
	};
}

std::vector<Pos> AStarPath::calculate_AStar_path(GameState* gs, Pos s, Pos e, BBox world_region, bool imperfect_knowledge) {
	perf_timer_begin(FUNCNAME);
	world_region = world_region.resized_within(
		BBox( Pos(), gs->get_level()->tiles().size() )
	);
	// No path unless both ends lie in the (map-clipped) region
	if (world_region.empty() || !world_region.contains(s) || !world_region.contains(e)) {
		perf_timer_end(FUNCNAME);
		return {};
	}
	int w = world_region.width(), h = world_region.height();
	if (nodes.width() != w || nodes.height() != h) {
		nodes.resize(Size(w, h));
		for (AStarNode& node : nodes) {
			node.stamp = 0;
		}
		stamp = 0;
	}
	if (++stamp == 0) { // wrapped around: old stamps could match again
		for (AStarNode& node : nodes) {
			node.stamp = 0;
		}
		stamp = 1;
	}

	// Solidity straight from the map, in region coordinates
	BoolGridRef solidity = gs->tiles().solidity_map();
	BoolGridRef seen = gs->tiles().previously_seen_map();
	Pos origin = world_region.left_top();
	auto solid = [&](int x, int y) {
		Pos xy(x + origin.x, y + origin.y);
		return (*solidity)[xy] && (!imperfect_knowledge || (*seen)[xy]);
	};

	s -= origin, e -= origin;
	int start = s.y * w + s.x, goal = e.y * w + e.x;
	AStarNode* node_data = &nodes.raw_get(0);

	std::vector<AStarEntry> heap;
	AStarEntryOrder order;
	auto visit = [&](int index, int previous, int g_score, int x, int y) {
		AStarNode& node = node_data[index];
		node.stamp = stamp;
		node.previous = previous;
		node.g_score = g_score;
		node.closed = false;
		int h_score = heurestic_distance(Pos(x, y), e);
		heap.push_back({g_score + h_score, h_score, g_score, index});
		std::push_heap(heap.begin(), heap.end(), order);
	};
	visit(start, -1, 0, s.x, s.y);

	while (!heap.empty()) {
		std::pop_heap(heap.begin(), heap.end(), order);
		AStarEntry top = heap.back();
		heap.pop_back();
		AStarNode& node = node_data[top.index];
		if (node.closed || top.g_score != node.g_score) {
			continue; // stale entry
		}
		if (top.index == goal) {
			break;
		}
		node.closed = true;

		int x = top.index % w, y = top.index / w;
		for (int yy = max(y - 1, 0); yy <= min(y + 1, h - 1); yy++) {
			for (int xx = max(x - 1, 0); xx <= min(x + 1, w - 1); xx++) {
				if ((xx == x && yy == y) || solid(xx, yy)) {
					continue;
				}
				bool diagonal = (xx != x && yy != y);
				// No cutting corners: a diagonal step needs both tiles beside it open
				if (diagonal && (solid(x, yy) || solid(xx, y))) {
					continue;
				}
				int index = yy * w + xx;
				AStarNode& neighbour = node_data[index];
				int g_score = node.g_score + (diagonal ? 114 : 100);
				if (neighbour.stamp != stamp) {
					visit(index, top.index, g_score, xx, yy);
				} else if (!neighbour.closed && g_score < neighbour.g_score) {
					visit(index, top.index, g_score, xx, yy);
				}
			}
		}
	}

	std::vector<Pos> positions;
	if (node_data[goal].stamp != stamp) {
		perf_timer_end(FUNCNAME);
		return positions; // unreachable
	}
	for (int index = goal; index != -1; index = node_data[index].previous) {
		Pos p((index % w) * TILE_SIZE + TILE_SIZE / 2, (index / w) * TILE_SIZE + TILE_SIZE / 2);
		positions.push_back(p + origin.scaled(TILE_SIZE));
	}
	reverse(positions.begin(), positions.end());
	perf_timer_end(FUNCNAME);

	return positions;
}

void draw_path(GameState* gs, std::vector<Pos>& path) {
	GameView& view = gs->view();
	for (int i = 0; i < path.size(); i++) {
		int draw_radius = 16;
		if (view.within_view(path[i].x - draw_radius, path[i].y - draw_radius,
				draw_radius * 2, draw_radius * 2)) {
			ldraw::draw_circle(Colour(255, 0, 0), on_screen(gs, path[i]),
					draw_radius);
		}
	}
}

