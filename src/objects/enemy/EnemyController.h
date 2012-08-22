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

#ifndef ENEMYCONTROLLER_H_
#define ENEMYCONTROLLER_H_

#include <vector>

#include "../../collision_avoidance/CollisionAvoidance.h"

#include "../../pathfind/pathfind.h"
#include "../../pathfind/astar_pathfind.h"

#include "../GameInst.h"
#include "EnemyInst.h"

struct EnemyBehaviour {
	int successful_hit_timer;
	int damage_taken_timer;
	EnemyBehaviour() :
			successful_hit_timer(successful_hit_timer), damage_taken_timer(
					damage_taken_timer) {

	}
};

struct EnemyOfInterest {
	EnemyInst* e;
	int closest_player_index;
	EnemyOfInterest(EnemyInst* e, int player_index) :
			e(e), closest_player_index(player_index) {
	}
};

namespace RVO {
struct RVOSimulator;
}

class GameLevelState;
class PlayerInst;

class EnemyController {
public:
	EnemyController(bool wander = true);
	~EnemyController();

	// Occurs for current level
	void pre_step(GameState* gs);
	void post_draw(GameState* gs);

	void register_enemy(GameInst* enemy);
	void deregister_enemy(EnemyInst* enemy);

	std::vector<EnemyInst*> enemies_in_level(level_id level);

	void shift_target(GameState* gs);

	void clear();

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);
private:
	void set_monster_headings(GameState* gs,
			std::vector<EnemyOfInterest> & eois);
	void update_position(GameState* gs, EnemyInst* e);
	void update_velocity(GameState* gs, EnemyInst* e);
	/*returns an index into the player_simids vector*/
	int find_player_to_target(GameState* gs, EnemyInst* e);
	void process_players(GameState* gs);
	void monster_wandering(GameState *gs, EnemyInst *e);
	void monster_follow_path(GameState *gs, EnemyInst *e);
	void monster_get_to_stairs(GameState *gs, EnemyInst *e);

	struct Combatant {
		CombatGameInst* inst;
		bool already_moved;
	};
	//Cleared every step
	struct InstCache {
		AStarPathFind astarcontext;
		std::vector<Combatant> combatants;
	};
	InstCache cache;

	CollisionAvoidance coll_avoid;
	bool monsters_wandering_flag;
};

#endif /* ENEMYCONTROLLER_H_ */
