/*
 * GameTiles.h
 *
 *  Created on: 2011-10-27
 *      Author: 100397561
 */

#ifndef GAMETILES_H_
#define GAMETILES_H_

#include <cstring>
#include "../procedural/roomgen.h"

static const int TILE_SIZE = 32;
class GameState;

class GameTiles {
public:
	int& get(int x, int y){ return tiles[y*width+x]; }
	bool seen(int x, int y){ return seen_tiles[y*width+x]; }
	//NOTE: unused for time being, remove in later version
	void draw(GameState* gs);
	//Called before object drawing
	void pre_draw(GameState* gs);
	//Called after object drawing
	void post_draw(GameState* gs);
	GameTiles(int width, int height, bool gen_level = true);
	~GameTiles(){
		delete[] tiles;
		delete[] seen_tiles;
	}
	int tile_width(){ return width; }
	int tile_height(){ return height; }
	void generate_level();
	RoomgenSettings& room_settings(){ return rs; }
private:
	char* seen_tiles;
	int* tiles;
	int width, height;
	RoomgenSettings rs;
};



#endif /* GAMETILES_H_ */
