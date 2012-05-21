/*
 * GameTiles.h
 * Part of GameLevelState.
 * Handles drawing of the tiles of the game, drawing of the fog-of-war, storage of tile information for a level
 */

#ifndef GAMETILES_H_
#define GAMETILES_H_

#include <cstring>
#include "../util/game_basic_structs.h"

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
    void step(GameState* gs);
	int tile_width(){ return width; }
	int tile_height(){ return height; }
	void clear (){
		memset(seen_tiles, 0, width*height);
		memset(tiles, 0, sizeof(int)*width*height);
	}
	void copy_to(GameTiles& t) const {
		t.width = width, t.height = height;
		memcpy(t.seen_tiles, seen_tiles, width*height);
		memcpy(t.tiles, tiles, sizeof(int)*width*height);
	}
private:
	char* seen_tiles;
	int* tiles;
	int width, height;
};



#endif /* GAMETILES_H_ */
