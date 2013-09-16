/*
 * AStarPath.h:
 *  Implements A* pathfinding
 */

#ifndef ASTAR_PATHFIND_H_
#define ASTAR_PATHFIND_H_

#include "objects/GameInst.h"
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
	AStarPath(){
		nodes = NULL;
		w = 0, h = 0;
	}

	~AStarPath(){
		delete[] nodes;
	}
	//outputs in real-world waypoints
	std::vector<Pos> calculate_AStar_path(GameState* gs, Pos s, Pos e, bool clear_results = true);

private:
	void initialize(GameState* gs);
	AStarNode* at(Pos xy){
		return &nodes[xy.y*w+xy.x];
	}
	AStarNode* nodes;
	bool can_cross(const Pos& s, const Pos& e);
	int w, h;

};

void draw_path(GameState* gs, std::vector<Pos>& path);

#endif
