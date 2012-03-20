/*
 * featuregen.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "featuregen.h"
#include "../world/GameState.h"
#include "../data/tile_data.h"

void generate_features(const FeatureGenSettings& fs, MTwist& mt, GeneratedLevel& level, GameState* gs){
	GameTiles& tiles = gs->tile_grid();
	tiles.clear();
	
	int tw = tiles.tile_width(), th = tiles.tile_height();
	int lw = level.width(), lh = level.height();

	int start_x = (tw-lw)/2;
	int start_y = (th-lh)/2;

	int end_x = start_x + lw;
	int end_y = start_y + lh;

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			Sqr& s = level.at(x-start_x, y-start_y);
			if (s.passable) {
				tiles.get(x,y) = TILE_FLOOR;
				if (s.roomID){
//					if (s.marking)
// 					tiles[ind] = TILE_MESH_0+s.marking;
				} else if (s.feature == SMALL_CORRIDOR){
					tiles.get(x,y) = TILE_CORRIDOR_FLOOR;
				}
			} else {
				tiles.get(x,y)= TILE_WALL;
				if (s.feature == SMALL_CORRIDOR){
					if (mt.rand(4) == 0){
						tiles.get(x,y) = TILE_STONE_WALL;
					}
				}
			}
		}
	}
	for(int n = 0; n < fs.nstairs_down; n++){
		Pos p = generate_location(mt, level);
		level.at(p).has_instance = true;
		p.x += start_x;
		p.y += start_y;
		
		tiles.get(p.x, p.y) = TILE_STAIR_DOWN;
	}
	
	for(int n = 0; n < fs.nstairs_up; n++){
		Pos p = generate_location(mt, level);
		level.at(p).has_instance = true;
		p.x += start_x;
		p.y += start_y;
		
		tiles.get(p.x, p.y) = TILE_STAIR_UP;
	}

}
