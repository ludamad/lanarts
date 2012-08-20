///*
// * EnemyController.h:
// *  Centralized location of all pathing decisions of enemies, with collision avoidance
// */
//
//#ifndef ENEMYCONTROLLER_H_
//#define ENEMYCONTROLLER_H_
//
//#include <vector>
//
//#include "../../collision_avoidance/CollisionAvoidance.h"
//
//#include "../../pathfind/pathfind.h"
//#include "../../pathfind/astar_pathfind.h"
//
//#include "../GameInst.h"
//#include "EnemyInst.h"
//
//struct EnemyOfInterest {
//	EnemyInst* e;
//	int closest_player_index;
//	EnemyOfInterest(EnemyInst* e, int player_index) :
//			e(e), closest_player_index(player_index) {
//	}
//};
//
//namespace RVO {
//struct RVOSimulator;
//}
//
//struct GameLevelState;
//class PlayerInst;
//
//class EnemyController {
//public:
//	EnemyController(bool wander = true);
//	~EnemyController();
//
//	// Occurs for current level
//	void pre_step(GameState* gs);
//	void post_draw(GameState* gs);
//
//	void register_enemy(GameInst* enemy);
//	void deregister_enemy(EnemyInst* enemy);
//
//	std::vector<EnemyInst*> enemies_in_level(level_id level);
//
//	void shift_target(GameState* gs);
//
//	void clear();
//
//	void serialize(GameState* gs, SerializeBuffer& serializer);
//	void deserialize(GameState* gs, SerializeBuffer& serializer);
//private:
//	void set_monster_headings(GameState* gs,
//			std::vector<EnemyOfInterest> & eois);
//	void update_position(GameState* gs, EnemyInst* e);
//	void update_velocity(GameState* gs, EnemyInst* e);
//	/*returns an index into the player_simids vector*/
//	int find_player_to_target(GameState* gs, EnemyInst* e);
//	void process_players(GameState* gs);
//	void monster_wandering(GameState *gs, EnemyInst *e);
//	void monster_follow_path(GameState *gs, EnemyInst *e);
//	void monster_get_to_stairs(GameState *gs, EnemyInst *e);
//
//	struct Combatant {
//		CombatGameInst* inst;
//		bool already_moved;
//	};
//	//Cleared every step
//	struct InstCache {
//		AStarPathFind astarcontext;
//		std::vector<Combatant> combatants;
//	};
//	InstCache cache;
//
//	CollisionAvoidance coll_avoid;
//	bool monsters_wandering_flag;
//};
//
//#endif /* ENEMYCONTROLLER_H_ */
