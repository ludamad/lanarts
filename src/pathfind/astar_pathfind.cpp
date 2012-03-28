/*
 * astar_pathfind.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: 100397561
 */


#include "astar_pathfind.h"
#include <algorithm>

#include "../data/tile_data.h"
#include "../world/GameState.h"

using namespace std;

class AStarOrderNodes {
public:
	bool operator()(AStarNode* a, AStarNode* b) {
		return (a->f_score > b->f_score);
	}
};

static int heurestic_distance(Pos s, Pos e){
	int h_diagonal = min(abs(s.x-e.x), abs(s.y-e.y));
	int h_straight = (abs(s.x-e.x) + abs(s.y-e.y));
	return 114 * h_diagonal + 100 * (h_straight - 2*h_diagonal);
/*	int dx = (s.x - e.x)*100;
	int dy = (s.y - e.y)*100;
	return sqrt(dx*dx+dy*dy);*/
}

void AStarPathFind::initialize(GameState* gs){
	if (!nodes){
		w = gs->width()/TILE_SIZE;
		h = gs->height()/TILE_SIZE;
		nodes = new AStarNode[w*h];
		for (int y = 0; y < h; y++){
			for (int x = 0; x < w; x++){
				int tilen = gs->tile_grid().get(x,y);
				TileEntry& entry = game_tile_data[tilen];
				nodes[y*w+x].solid = entry.solid;
			}
		}
	}
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			nodes[y*w+x].openset = false;
			nodes[y*w+x].closedset = false;
			nodes[y*w+x].previous = NULL;
		}
	}
}

bool AStarPathFind::can_cross(const Pos& s, const Pos& e){
	return !at(s.x, e.y)->solid && !at(e.x, s.y)->solid;
}
/*Used to make sure all interpolated directions are possible*/
std::vector<Pos> AStarPathFind::calculate_AStar_path(GameState *gs, int sx, int sy, int ex, int ey){
	initialize(gs);

	AStarOrderNodes orderfunc;
	std::vector<AStarNode*> heap;

	AStarNode* start = at(sx,sy);

	start->g_score = 0;
	start->h_score = heurestic_distance(Pos(sx,sy), Pos(ex,ey));
	start->f_score = start->h_score;
	start->openset = true;
	start->previous = NULL;
	start->closedset = false;

	heap.push_back(start);
	int iters = 0;
	while (!heap.empty()){
		iters++;
		//Node with minimum f_score
		AStarNode* node = heap[0];
		node->openset = false;
		node->closedset = true;

		std::pop_heap(heap.begin(), heap.end(), orderfunc);
		heap.pop_back();

		int x = (node - nodes) % w;
		int y = (node - nodes) / w;

		for (int yy = y-1; yy <= y+1; yy++){
			if (yy < 0 || yy >= h) continue;
			for (int xx = x-1; xx <= x+1; xx++){
				if (xx < 0 || xx >= w) continue;

				AStarNode* neighbour = at(xx,yy);
				if (neighbour->closedset) continue;
				if (!can_cross(Pos(x,y), Pos(xx,yy))) continue;

				int dx = x-xx, dy = y-yy;
				int g_score = node->g_score + (abs(dx) == abs(dy) ? 114 : 100);
				if (!neighbour->solid && !neighbour->openset){
					heap.push_back(neighbour);
					std::push_heap(heap.begin(), heap.end(), orderfunc);
					neighbour->openset = true;
					neighbour->previous = node;
					neighbour->h_score = heurestic_distance(Pos(xx,yy), Pos(ex,ey));
					neighbour->g_score = g_score;
					neighbour->f_score = neighbour->g_score + neighbour->h_score;
				} else if (!neighbour->solid && neighbour->g_score > g_score){
					neighbour->previous = node;
					neighbour->g_score = g_score;
					neighbour->f_score = neighbour->g_score + neighbour->h_score;
				}
			}

		}
	}
	std::vector<Pos> positions;
	AStarNode* traverse = at(ex,ey);
	while (traverse){
		int x = (traverse - nodes) % w;
		int y = (traverse - nodes) / w;
		traverse = traverse->previous;
		Pos p(x*TILE_SIZE+TILE_SIZE/2, y*TILE_SIZE+TILE_SIZE/2);
		positions.push_back(p);
	}
	reverse(positions.begin(), positions.end());
	return positions;
}

void draw_path(GameState* gs, std::vector<Pos>& path){
	GameView& view = gs->window_view();
	for (int i = 0; i < path.size(); i++){
		int draw_radius = 16;
		if (view.within_view(path[i].x-draw_radius, path[i].y-draw_radius,
				draw_radius*2, draw_radius*2)){
			gl_draw_circle(view, path[i].x, path[i].y, draw_radius, Colour(255,0,0));
		}
	}
}

