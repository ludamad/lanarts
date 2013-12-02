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

#include "gheap.h"

using namespace std;

class AStarOrderNodes {
public:
	bool operator()(AStarNode* a, AStarNode* b) const {
		if (a->f_score == b->f_score) {
			//Maintain strict-weak-ordering
			return a > b;
		}
		return (a->f_score > b->f_score);
	}
};

static inline int heurestic_distance(const Pos& s, const Pos& e) {
	int h_diagonal = min(abs(s.x - e.x), abs(s.y - e.y));
	int h_straight = (abs(s.x - e.x) + abs(s.y - e.y));
	return 114 * h_diagonal + 100 * (h_straight - 2 * h_diagonal);
}

void AStarPath::initialize(GameState* gs, BBox world_region, bool imperfect_knowledge) {
	perf_timer_begin(FUNCNAME);
	BoolGridRef grid = gs->tiles().solidity_map();
	nodes.resize(world_region.size());
	int width = world_region.width();
	if (imperfect_knowledge) {
		BoolGridRef seen = gs->tiles().previously_seen_map();
		FOR_EACH_BBOX_XY(world_region, xy) {
			AStarNode& node = nodes[xy - world_region.left_top()];
			node.openset = false;
			node.closedset = false;
			node.previous = NULL;
			node.solid = ((*seen)[xy] && (*grid)[xy]);
		}
	} else {
		FOR_EACH_BBOX_XY(world_region, xy) {
			AStarNode& node = nodes[xy - world_region.left_top()];
			node.openset = false;
			node.closedset = false;
			node.previous = NULL;
			node.solid = (*grid)[xy];
		}
	}
	perf_timer_end(FUNCNAME);
}

bool AStarPath::can_cross(const Pos& s, const Pos& e) {
	return !nodes[Pos(s.x, e.y)].solid && !nodes[Pos(e.x, s.y)].solid;
}

static std::vector<AStarNode*>::iterator find_heap_position(
		std::vector<AStarNode*>& heap, AStarNode* node) {
	for (std::vector<AStarNode*>::iterator it = heap.begin(); it != heap.end();
			++it) {
		if (*it == node) {
			return it;
		}
	}
	LANARTS_ASSERT(false);
	return heap.end();
}

/*Used to make sure all interpolated directions are possible*/
std::vector<Pos> AStarPath::calculate_AStar_path(GameState* gs, Pos s, Pos e, BBox world_region, bool imperfect_knowledge, bool clear_results) {
	perf_timer_begin(FUNCNAME);
	world_region = world_region.resized_within(
		BBox( Pos(), gs->get_level()->tiles().size() )
	);
	if (clear_results) {
		initialize(gs, world_region, imperfect_knowledge);
	}
	AStarNode* start_node = &nodes.raw_get(0);
	// Adjust the coordinates to be within our region
	s -= world_region.left_top(), e -= world_region.left_top();

	AStarOrderNodes orderfunc;
	std::vector<AStarNode*> heap;

	AStarNode* start = &nodes[s];

	int w = world_region.width(), h = world_region.height();
	start->g_score = 0;
	start->h_score = heurestic_distance(s, e);
	start->f_score = start->h_score;
	start->openset = true;
	start->previous = NULL;
	start->closedset = false;

	heap.push_back(start);
	int iters = 0;
	while (!heap.empty()) {
		iters++;
		//Node with minimum f_score
		AStarNode* node = heap[0];
		node->openset = false;
		node->closedset = true;

		gheap<>::pop_heap(heap.begin(), heap.end(), orderfunc);
		heap.pop_back();

		int x = (node - start_node) % w;
		int y = (node - start_node) / w;

		for (int yy = y - 1; yy <= y + 1; yy++) {
			if (yy < 0 || yy >= h)
				continue;
			for (int xx = x - 1; xx <= x + 1; xx++) {
				if (xx < 0 || xx >= w)
					continue;

				AStarNode* neighbour = &nodes[Pos(xx, yy)];
				if (neighbour->closedset)
					continue;
				if (!can_cross(Pos(x, y), Pos(xx, yy)))
					continue;

				int dx = x - xx, dy = y - yy;
				int g_score = node->g_score + (abs(dx) == abs(dy) ? 114 : 100);
				if (!neighbour->solid && !neighbour->openset) {
					neighbour->openset = true;
					neighbour->previous = node;
					neighbour->h_score = heurestic_distance(Pos(xx, yy), e);
					neighbour->g_score = g_score;
					neighbour->f_score = neighbour->g_score
							+ neighbour->h_score;
					heap.push_back(neighbour);
					gheap<>::push_heap(heap.begin(), heap.end(), orderfunc);
				} else if (!neighbour->solid && neighbour->g_score > g_score) {
					neighbour->previous = node;
					neighbour->g_score = g_score;
					neighbour->f_score = neighbour->g_score
							+ neighbour->h_score;

					//TODO: Reevaluate if this really needs to be O(N) + O(logN)
					gheap<>::restore_heap_after_item_increase(heap.begin(),
							find_heap_position(heap, neighbour), orderfunc);
				}
			}

		}
	}
	std::vector<Pos> positions;
	AStarNode* traverse = &nodes[e];
	while (traverse) {
		int x = (traverse - start_node) % w;
		int y = (traverse - start_node) / w;
		traverse = traverse->previous;
		Pos p(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
		positions.push_back(p + world_region.left_top().scaled(TILE_SIZE));
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

