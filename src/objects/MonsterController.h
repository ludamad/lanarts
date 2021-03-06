/*
 * MonsterController.h:
 *  Centralized location of all pathing decisions of monsters, with collision avoidance
 */

#ifndef MONSTERCONTROLLER1_H_
#define MONSTERCONTROLLER1_H_

#include <vector>

#include "collision_avoidance/CollisionAvoidance.h"

#include "pathfind/AStarPath.h"

#include "pathfind/FloodFillPaths.h"
#include <lcommon/SerializeBuffer.h>

#include "objects/GameInst.h"
#include "objects/EnemyInst.h"

struct EnemyOfInterest {
	EnemyInst* e;
	int actor_id;
	int distance;
	EnemyOfInterest(EnemyInst* e, int actor_id, int distance) :
			e(e), actor_id(actor_id), distance(distance) {
	}
	bool operator<(const EnemyOfInterest& eoi) const {
		return distance < eoi.distance;
	}
};

namespace RVO {
struct RVOSimulator;
}

class GameMapState;
class PlayerInst;

class MonsterController {
public:
	MonsterController(bool wander = true);
	~MonsterController();

	const std::vector<obj_id>& monster_ids() {
		return mids;
	}

	void pre_step(GameState* gs);
	void post_draw(GameState* gs);

	void register_enemy(GameInst* enemy);

	size_t number_monsters() {
		return mids.size();
	}

	void clear();
	//Copy everything but RVO::Simulator
	void partial_copy_to(MonsterController& mc) const;
	//Fill out RVO::Simulator
	void finish_copy(GameMapState* level);

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

private:
	void set_monster_headings(GameState* gs,
			std::vector<EnemyOfInterest> & eois);
	void update_position(GameState* gs, EnemyInst* e);
	void update_velocity(GameState* gs, EnemyInst* e);
	/*returns an index into the player_simids vector*/
	CombatGameInst* find_actor_to_target(GameState* gs, EnemyInst* e);
	void monster_wandering(GameState *gs, EnemyInst *e);
	void monster_follow_path(GameState *gs, EnemyInst *e);
	void monster_get_to_stairs(GameState *gs, EnemyInst *e);

	AStarPath astarcontext;

	std::vector<PlayerInst*> players;
	std::vector<obj_id> mids;

	bool monsters_wandering_flag;
};

#endif /* MONSTERCONTROLLER_H_ */
