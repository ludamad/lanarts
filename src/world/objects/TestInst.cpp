/*
 * TestInst.cpp
 *
 *  Created on: Feb 6, 2012
 *      Author: 100397561
 */

#include "TestInst.h"
#include "../../pathfind/pathfind.h"
#include "../../data/tile_data.h"
#include "../GameState.h"

TestInst::~TestInst() {
}


void TestInst::step(GameState* gs){

}

void TestInst::draw(GameState* gs){
//	GameView& view = gs->window_view();
//	GameTiles& tile = gs->tile_grid();
//
//	int min_tilex, min_tiley;
//	int max_tilex, max_tiley;
//
//	view.min_tile_within(min_tilex, min_tiley);
//	view.max_tile_within(max_tilex, max_tiley);
//
//	int w = max_tilex-min_tilex, h = max_tiley-min_tiley;
//
//	PathingNode* path = new PathingNode[w*h];
//	for (int y = 0; y < h; y++){
//		for (int x = 0; x < w; x++){
//			PathingNode* node = &path[y*w+x];
//			node->solid = tile.get(x+min_tilex, y+min_tiley) <= TILE_STONE_WALL;
//			node->open = true;
//		}
//	}
//	GameInst* player = gs->player_obj();
//	int px = player->last_x/TILE_SIZE, py = player->last_y/TILE_SIZE;
//	floodfill(path, w, h, px-min_tilex, py-min_tiley);
//	for (int y = 0; y < h; y++){
//		for (int x = 0; x < w; x++){
//			PathingNode* node = &path[y*w+x];
//			if (!node->solid)
//			gl_printf(gs->primary_font(), Colour(255,255,255), (x+min_tilex)*TILE_SIZE-view.x, (y+min_tiley)*TILE_SIZE-view.y,
//					"%d", node->prev_node.distance);
//		}
//	}
//
//
//	delete[] path;
}
