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

struct AStarNode {
	AStarNode* previous;
	int g_score, h_score, f_score;
	bool openset;
	bool closedset;
	bool solid;
};

class AStarPath {
public:
	//outputs in real-world waypoints
	std::vector<Pos> calculate_AStar_path(GameState* gs, Pos s, Pos e,
			BBox world_region, bool imperfect_knowledge = false, bool clear_results = true);

	static BBox surrounding_region(Pos s, Pos e, Size padding) {
		int sx = std::min(s.x, e.x), sy = std::min(s.y, e.y);
		int ex = std::max(s.x, e.x), ey = std::max(s.y, e.y);
		return BBox(Pos(sx - padding.w / 2, sy - padding.h / 2), Size(ex - sx, ey - sy) + padding);
	}
private:
	void initialize(GameState* gs, BBox world_region, bool imperfect_knowledge);
	bool can_cross(const Pos& s, const Pos& e);
	Grid<AStarNode> nodes;
};

void draw_path(GameState* gs, std::vector<Pos>& path);

#endif
