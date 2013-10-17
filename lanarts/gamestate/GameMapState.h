/*
 * GameMapState.h:
 *  Contains game map (level) data
 */

#ifndef GAMEMAPSTATE_H_
#define GAMEMAPSTATE_H_

#include <vector>

#include <lcommon/geometry.h>

#include "collision_avoidance/CollisionAvoidance.h"
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

	obj_id add_instance(GameState* gs, GameInst* inst);

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void step(GameState* gs);
	void draw(GameState* gs, bool reveal_all = false);

	std::string& label() {
		return _label;
	}

	// TODO: De-duplicate this (same logic in GameState, for now)

	bool tile_radius_test(int x, int y, int rad, bool issolid = true,
			int ttype = -1, Pos* hitloc = NULL) {
		return tiles().radius_test(Pos(x, y), rad, issolid, ttype, hitloc);
	}

	int object_radius_test(GameInst* obj, GameInst** objs, int obj_cap,
			col_filterf f, int x, int y, int radius) {
		return game_inst_set().object_radius_test(obj, objs, obj_cap, f, x, y, radius);
	}

	bool solid_test(GameInst* obj, int x, int y, int radius = -1) {
		return solid_test(obj, NULL, 0, NULL, x, y, radius);
	}

	bool solid_test(GameInst* obj, GameInst** objs = NULL, int obj_cap = 0,
			col_filterf f = NULL, int x = -1, int y = -1, int radius = -1) {
		int lx = (x == -1 ? obj->x : x), ly = (y == -1 ? obj->y : y);
		return tile_radius_test(lx, ly, radius == -1 ? obj->radius : radius)
				|| object_radius_test(obj, objs, obj_cap, f, x, y, radius);
	}

public:
	std::vector<GameRoomPortal> exits, entrances;
private:
	std::string _label;
	level_id _levelid;
	int _steps_left;
	Size _size;
	GameTiles _tiles;
	GameInstSet _inst_set;
	MonsterController _monster_controller;
	CollisionAvoidance _collision_avoidance;
	/* Used to store dynamic drawable information */
	LuaDrawableQueue _drawable_queue;

	bool _is_simulation;
};

#endif /* GAMEMAPSTATE_H_ */
