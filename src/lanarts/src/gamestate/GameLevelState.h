/*
 * GameRoomState.h:
 *  Contains game room data (room == dungeon level == dungeon floor)
 */

#ifndef GAMEROOMSTATE_H_
#define GAMEROOMSTATE_H_

#include <vector>

#include <lcommon/geometry.h>

#include "collision_avoidance/CollisionAvoidance.h"
#include "dungeon_generation/GeneratedLevel.h"
#include "objects/enemy/MonsterController.h"
#include "pathfind/WanderMap.h"

#include "GameInstSet.h"

#include "GameTiles.h"

#include "PlayerData.h"

class GameState;
class SerializeBuffer;

struct GameRoomPortal {
	Pos entrancesqr;
	Pos exitsqr; //0,0 if undecided
	GameRoomPortal(const Pos& entrance = Pos(), const Pos& exit = Pos()) :
					entrancesqr(entrance),
					exitsqr(exit) {
	}
};

class GameRoomState {
public:
	GameRoomState(level_id levelid, const Size& size,
			bool wandering_flag = true, bool is_simulation = false);
	~GameRoomState();

	int room_within(const Pos& p);
	void copy_to(GameRoomState & level) const;
	GameRoomState* clone() const;

	int width() const {
		return _size.w;
	}
	int height() const {
		return _size.h;
	}

	GameTiles& tiles() {
		return _tiles;
	}

	GameInstSet& game_inst_set() {
		return _inst_set;
	}
	int tile_width() const {
		return _size.w / TILE_SIZE;
	}
	int tile_height() const {
		return _size.h / TILE_SIZE;
	}

	level_id id() const {
		return _levelid;
	}

	MonsterController& monster_controller() {
		return _monster_controller;
	}

	CollisionAvoidance& collision_avoidance() {
		return _collision_avoidance;
	}

	WanderMap& wander_map() {
		return _wander_map;
	}

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void step(GameState* gs);

public:
	std::vector<GameRoomPortal> exits, entrances;
	std::vector<Room> rooms;
private:

	level_id _levelid;
	int _steps_left;
	Size _size;
	GameTiles _tiles;
	GameInstSet _inst_set;
	MonsterController _monster_controller;
	CollisionAvoidance _collision_avoidance;
	WanderMap _wander_map;

	bool _is_simulation;
};

#endif /* GAMEROOMSTATE_H_ */
