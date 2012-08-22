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

namespace RVO {
struct RVOSimulator;
}

struct GameLevelState;
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

CombatGameInst* find_closest_hostile(GameState* gs, CombatGameInst* obj,
		const std::vector<CombatGameInst*>& candidates);

obj_id find_closest_hostile(GameState* gs, CombatGameInst* obj,
		const std::vector<CombatGameInst*>& candidates);


int MonsterController::find_player_to_target(GameState* gs, EnemyInst* e) {
	//Use a 'GameView' object to make use of its helper methods
	GameView view(0, 0, PLAYER_PATHING_RADIUS * 2, PLAYER_PATHING_RADIUS * 2,
			gs->width(), gs->height());

	//Determine which players we are currently in view of
	BBox ebox = e->bbox();
	int mindistsqr = HUGE_DISTANCE;
	int closest_player_index = -1;
	for (int i = 0; i < players.size(); i++) {
		PlayerInst* player = players[i];
		bool isvisible = gs->object_visible_test(e, player, false);
		if (isvisible)
			((PlayerInst*)player)->rest_cooldown() = REST_COOLDOWN;
		view.sharp_center_on(player->x, player->y);
		bool chasing = e->behaviour().chase_timeout > 0
				&& player->id == e->behaviour().chasing_player;
		if (view.within_view(ebox) && (chasing || isvisible)) {
			e->behaviour().current_action = EnemyBehaviour::CHASING_PLAYER;

			int dx = e->x - player->x, dy = e->y - player->y;
			int distsqr = dx * dx + dy * dy;
			if (distsqr > 0 /*overflow check*/&& distsqr < mindistsqr) {
				mindistsqr = distsqr;
				closest_player_index = i;
			}
		}
	}
	return closest_player_index;
}

#endif /* ENEMYCONTROLLER_H_ */
