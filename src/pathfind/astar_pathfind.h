/*
 * astar_pathfind.h
 *
 *  Created on: Mar 27, 2012
 *      Author: 100397561
 */

#ifndef ASTAR_PATHFIND_H_
#define ASTAR_PATHFIND_H_
#include "../world/objects/GameInst.h"
#include "../procedural/GeneratedLevel.h"
#include <vector>

struct AStarNode {
	AStarNode* previous;
	int g_score, h_score, f_score;
	bool openset;
	bool closedset;
	bool solid;
};
class AStarPathFind{
public:
	AStarPathFind(){
		nodes = NULL;
		w = 0, h = 0;
	}

	~AStarPathFind(){
		delete[] nodes;
	}
	//outputs in real-world waypoints
	std::vector<Pos> calculate_AStar_path(GameState *gs, int sx, int sy, int ex, int ey);

private:
	void initialize(GameState* gs);
	AStarNode* at(int x, int y){
		return &nodes[y*w+x];
	}
	AStarNode* nodes;
	bool can_cross(const Pos& s, const Pos& e);
	int w, h;

};

void draw_path(GameState* gs, std::vector<Pos>& path);

#endif
