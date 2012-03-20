/*
 * GameTiles.h
 *
 *  Created on: 2011-10-27
 *      Author: 100397561
 */

#ifndef GAMETILES_H_
#define GAMETILES_H_

#include <cstring>

static const int TILE_SIZE = 32;
class GameState;

class GameTiles {
public:
	int& get(int x, int y){ return tiles[y*width+x]; }
	int* tile_array(){return tiles; }
	bool seen(int x, int y){ return seen_tiles[y*width+x]; }
	//NOTE: unused for time being, remove in later version
	void draw(GameState* gs);
	//Called before object drawing
	void pre_draw(GameState* gs);
	//Called after object drawing
	void post_draw(GameState* gs);
	GameTiles(int width, int height);
	~GameTiles(){
		delete[] tiles;
		delete[] seen_tiles;
	}
	int tile_width(){ return width; }
	int tile_height(){ return height; }
	void clear (){
		memset(seen_tiles, 0, width*height);
		memset(tiles, 0, sizeof(int)*width*height);
	}
private:
	char* seen_tiles;
	int* tiles;
	int width, height;
};



#endif /* GAMETILES_H_ */
