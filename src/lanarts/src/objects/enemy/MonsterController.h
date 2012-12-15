/*
 * MonsterController.h:
 *  Centralized location of all pathing decisions of monsters, with collision avoidance
 */

#ifndef MONSTERCONTROLLER_H_
#define MONSTERCONTROLLER_H_

#include <vector>

#include "../../collision_avoidance/CollisionAvoidance.h"

#include "../../pathfind/astar_pathfind.h"

#include "../../pathfind/pathfind.h"
#include <lcommon/SerializeBuffer.h>

#include "../GameInst.h"
#include "EnemyInst.h"

struct EnemyOfInterest {
	EnemyInst* e;
	int closest_player_index;
	int distance;
	EnemyOfInterest(EnemyInst* e, int player_index, int distance) :
			e(e), closest_player_index(player_index), distance(distance) {
	}
	bool operator<(const EnemyOfInterest& eoi) const {
		return distance < eoi.distance;
	}
};

namespace RVO {
struct RVOSimulator;
}

class GameLevelState;
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
	void finish_copy(GameLevelState* level);

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

private:
	void set_monster_headings(GameState* gs,
			std::vector<EnemyOfInterest> & eois);
	void update_position(GameState* gs, EnemyInst* e);
	void update_velocity(GameState* gs, EnemyInst* e);
	/*returns an index into the player_simids vector*/
	int find_player_to_target(GameState* gs, EnemyInst* e);
	void monster_wandering(GameState *gs, EnemyInst *e);
	void monster_follow_path(GameState *gs, EnemyInst *e);
	void monster_get_to_stairs(GameState *gs, EnemyInst *e);

	AStarPathFind astarcontext;

	std::vector<PlayerInst*> players;
	std::vector<obj_id> mids;

	bool monsters_wandering_flag;
};

#endif /* MONSTERCONTROLLER_H_ */
