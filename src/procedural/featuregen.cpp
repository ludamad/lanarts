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

	for (int i = 0; i < level.rooms().size(); i++){
		if (gs->rng().rand(3) != 0) continue;
		Region r = level.rooms()[i].room_region;
		/*

		int rx = r.x + gs->rng().rand(1, r.w-1);
		int ry = r.y + gs->rng().rand(1, r.h-1);
		int rw = gs->rng().rand(r.w);
		int rh = gs->rng().rand(r.h);

		if (rx + rw >= r.x+r.w) rw = r.x+r.w - 1 - rx;
		if (ry + rh >= r.y+r.h) rh = r.y+r.h - 1 - ry;*/
		int rx = r.x, rw = r.w;
		int ry = r.y, rh = r.h;


		for (int y = ry; y < ry+rh; y++) {
			for (int x = rx; x < rx+rw; x++) {
				if (tiles.get(x+start_x, y+start_y) == TILE_FLOOR)
				tiles.get(x+start_x,y+start_y) = TILE_FLOOR2;
			}
		}
	}
	for(int n = 0; n < fs.nstairs_down; n++){
		Pos p = generate_location(mt, level);
		level.at(p).has_instance = true;

		p.x += start_x;
		p.y += start_y;

		tiles.get(p.x, p.y) = TILE_STAIR_DOWN;

		gs->level()->entrances.push_back(GameLevelPortal(p, Pos(0,0)));
	}

	for(int n = 0; n < fs.nstairs_up; n++){
		Pos p = generate_location(mt, level);
		level.at(p).has_instance = true;
		
		int x = p.x - 2, y = p.y - 2;
		int ex = p.x + 3, ey = p.y + 3;
		x = std::max(0, x), y = std::max(0,y);
		ex = std::min(ex, level.width()), ey = std::min(ey,level.height());
		for (int yy = y; yy < ey; yy++) {
			for (int xx = x; xx < ex; xx++) {
				level.at(xx,yy).near_entrance = true;
//				if (xx != x || yy != y)//For visual confirmation
//					tiles.get(xx+start_x,yy+start_y) = TILE_MESH_0;
			}
		}

		p.x += start_x;
		p.y += start_y;
		
		tiles.get(p.x, p.y) = TILE_STAIR_UP;
		gs->level()->exits.push_back(GameLevelPortal(p, Pos(0,0)));
	}

}
