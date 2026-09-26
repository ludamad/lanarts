/*
 * AStarPath.h:
 *  Implements A* pathfinding
 */

#ifndef ASTAR_PATHFIND_H_
#define ASTAR_PATHFIND_H_

#include "lanarts_defines.h"
#include "objects/GameInst.h"
#include <lcommon/Grid.h>
#include <vector>

// Per-tile search state. It only holds for the search whose stamp it carries, so a new search
// needs no clearing pass over the region.
struct AStarNode {
	unsigned int stamp = 0;
	int previous = -1; // index of the node the best path arrives from
	int g_score = 0;
	bool closed = false;
};

class AStarPath {
public:
	// Outputs real-world waypoints from s to e (tile positions), searching within world_region
	// (clipped to the map). Empty when either end is outside it or e can't be reached. With
	// imperfect_knowledge, tiles the player hasn't seen count as open.
	std::vector<Pos> calculate_AStar_path(GameState* gs, Pos s, Pos e,
			BBox world_region, bool imperfect_knowledge = false);

	static BBox surrounding_region(Pos s, Pos e, Size padding) {
		int sx = std::min(s.x, e.x), sy = std::min(s.y, e.y);
		int ex = std::max(s.x, e.x), ey = std::max(s.y, e.y);
		return BBox(Pos(sx - padding.w / 2, sy - padding.h / 2), Size(ex - sx, ey - sy) + padding);
	}
private:
	Grid<AStarNode> nodes;
	unsigned int stamp = 0;
};

void draw_path(GameState* gs, std::vector<Pos>& path);

#endif
