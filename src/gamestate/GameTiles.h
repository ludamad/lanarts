/*
 * GameTiles.h
 * Part of GameLevelState.
 * Handles drawing of the tiles of the game, drawing of the fog-of-war, storage of tile information for a level
 */

#ifndef GAMETILES_H_
#define GAMETILES_H_

#include <cstring>
#include <vector>
#include "../lanarts_defines.h"

class GameState;
class SerializeBuffer;

class GameTiles {
public:
	GameTiles(int width, int height);
	~GameTiles();

	void step(GameState* gs);

	void pre_draw(GameState* gs);
	void post_draw(GameState* gs);

	int tile_width();
	int tile_height();

	Tile& get(int x, int y);

	void set_solid(int x, int y, bool solid);

	bool is_seen(int x, int y);
	bool is_solid(int x, int y);

	void mark_all_seen();

	void clear();
	void copy_to(GameTiles& t) const;
	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
private:
	struct TileState {
		bool seen, solid;
		Tile tile;
		TileState() :
				seen(0), solid(1) {
		}
	};
	int width, height;
	std::vector<TileState> tiles;
};

#endif /* GAMETILES_H_ */
