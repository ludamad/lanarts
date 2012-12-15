/*
 * EnemyController.h:
 *  Centralized location of all pathing decisions of enemies, with collision avoidance
 *  Enemy movement is dictated by the following logic:
 *    - Sort all enemies based on distance to a player
 *    - Decide an action, such as wandering or chasing a hostile creature (eg player, but same logic applies for allies)
 *    - If an enemy in the same mob has chosen to chase a hostile, and you are wandering,
 *    	chase that hostile instead
 *    		- If too far to chase the hostile, attempt to get closer to mob closest to player
 *    - Do not move forward if you would overlap with an enemy whose move has already been decided
 *    	AND you are both chasing the same hostile (This is a special case for tight corridors)
 */

// TODO: this is a pending rewrite of MonsterController

#ifndef ENEMYCONTROLLER_H_
#define ENEMYCONTROLLER_H_

#include <vector>

#include "collision_avoidance/CollisionAvoidance.h"

#include "pathfind/astar_pathfind.h"
#include "pathfind/pathfind.h"

#include "../GameInst.h"
#include "EnemyInst.h"

namespace RVO {
struct RVOSimulator;
}

class GameLevelState;
class PlayerInst;

class EnemyController {
public:
	// Occurs for current level
	void pre_step(GameState* gs);
	void post_draw(GameState* gs);

	std::vector<EnemyInst*> enemies_in_level(level_id level);

	void clear();

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);
private:

	// These are set every step and do not need to be serialized
	std::vector<CombatGameInst*> enemies;
	std::vector<CombatGameInst*> players;
};
#endif /* ENEMYCONTROLLER_H_ */
