/*
 * GameLevelState.h:
 *  Contains game level state information
 */

#ifndef GAMELEVELSTATE_H_
#define GAMELEVELSTATE_H_
#include <vector>
#include "controllers/MonsterController.h"
#include "controllers/PlayerController.h"
#include "GameInstSet.h"
#include "GameTiles.h"
#include "../procedural/GeneratedLevel.h"

struct GameLevelPortal {
	Pos entrancesqr;
	Pos exitsqr; //0,0 if undecided
	GameLevelPortal(Pos entrance, Pos exit) :
			entrancesqr(entrance), exitsqr(exit) {
	}
};

struct GameLevelState {
	GameLevelState(int roomid, int branch, int level, int w, int h, bool wandering_flag = true,
			bool is_simulation = false);
	~GameLevelState();

	int room_within(const Pos& p);
	void copy_to(GameLevelState & level) const;
	GameLevelState *clone() const;

	int tile_width() { return width / TILE_SIZE; }
	int tile_height() { return height / TILE_SIZE; }

	std::vector<GameLevelPortal> exits, entrances;
	std::vector<Room> rooms;
	int roomid;
	int steps_left;
	int branch_number, level_number;
	int width, height;
	GameTiles tiles;
	GameInstSet inst_set;
	MonsterController mc;
	PlayerController pc;

	bool is_simulation;
};

#endif /* GAMELEVELSTATE_H_ */
