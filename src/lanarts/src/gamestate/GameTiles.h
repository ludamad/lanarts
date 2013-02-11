/*
 * GameTiles.h:
 *   Handles drawing of the tiles of the game, drawing of the fog-of-war,
 *   storage of tile information for a level. Stores if a tile has already
 *   been seen.
 */

#ifndef GAMETILES_H_
#define GAMETILES_H_

#include <cstring>
#include <vector>

#include "lanarts_defines.h"

#include "util/Grid.h"

#include "pathfind/SolidityGridRef.h"

class GameState;
class SerializeBuffer;

struct Pos;

class GameTiles {
public:
	GameTiles(const Size& size);
	~GameTiles();

	void step(GameState* gs);

	void pre_draw(GameState* gs);
	void post_draw(GameState* gs);

	int tile_width();
	int tile_height();

	Size size() const;
	Tile& get(const Pos& xy);

	void set_solid(const Pos& xy, bool solid);
	void set_seethrough(const Pos& xy, bool seethrough);

	bool is_seethrough(const Pos& xy);
	bool is_seen(const Pos& xy);
	bool is_solid(const Pos& xy);

	void mark_all_seen();

	bool radius_test(const Pos& xy, int rad, bool issolid = true, int ttype = -1,
			Pos* hitloc = NULL);
	void clear();
	void copy_to(GameTiles& t) const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
private:
	struct TileState {
		bool seen, seethrough;
		Tile tile;
		TileState() :
				seen(false), seethrough(true) {
		}
	};

	/* Stores whether a given tile is solid */
	SolidityGridRef _solidity;

	/* Stores information about tiles, such as if they have
	 * been seen yet, and if they are see-through */
	Grid<TileState> _tiles;
};

#endif /* GAMETILES_H_ */
