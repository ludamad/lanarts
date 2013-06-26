/*
 * GameMapState.h:
 *  Contains game map (level) data
 */

#ifndef GAMEMAPSTATE_H_
#define GAMEMAPSTATE_H_

#include <vector>

#include <lcommon/geometry.h>

#include "collision_avoidance/CollisionAvoidance.h"
#include "dungeon_generation/GeneratedRoom.h"
#include "objects/enemy/MonsterController.h"
#include "pathfind/WanderMap.h"

#include "LuaDrawableQueue.h"
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

class GameMapState {
public:
	GameMapState(level_id levelid, const Size& size,
			bool wandering_flag = true, bool is_simulation = false);
	~GameMapState();

	int room_within(const Pos& p);
	void copy_to(GameMapState & level) const;
	GameMapState* clone() const;

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

	LuaDrawableQueue& drawable_queue() {
		return _drawable_queue;
	}

	WanderMap& wander_map() {
		return _wander_map;
	}

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void step(GameState* gs);

public:
	std::vector<GameRoomPortal> exits, entrances;
	std::vector<RoomRegion> rooms;
private:

	level_id _levelid;
	int _steps_left;
	Size _size;
	GameTiles _tiles;
	GameInstSet _inst_set;
	MonsterController _monster_controller;
	CollisionAvoidance _collision_avoidance;
	WanderMap _wander_map;
	/* Used to store dynamic drawable information */
	LuaDrawableQueue _drawable_queue;

	bool _is_simulation;
};

#endif /* GAMEMAPSTATE_H_ */
