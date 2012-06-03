/*
 * GameTiles.h
 * Part of GameLevelState.
 * Handles drawing of the tiles of the game, drawing of the fog-of-war, storage of tile information for a level
 */

#ifndef GAMETILES_H_
#define GAMETILES_H_

#include <cstring>
#include <vector>
#include "../util/game_basic_structs.h"

class GameState;

class GameTiles {
public:
	GameTiles(int width, int height);
	~GameTiles();

	void step(GameState *gs);

	void pre_draw(GameState *gs);
	void post_draw(GameState *gs);

	int tile_width();
	int tile_height();

	Tile* tile_array();

	Tile& get(int x, int y);

	void set_solid(int x, int y, bool solid);

	bool is_seen(int x, int y);
	bool is_solid(int x, int y);


	void clear();
	void copy_to(GameTiles& t) const;
private:
	char* seen_tiles;
	Tile* tiles;
	int width, height;
	std::vector<bool> solid_tiles;
};

#endif /* GAMETILES_H_ */
