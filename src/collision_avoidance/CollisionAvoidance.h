/*
 * CollisionAvoidance.h:
 *  A stable interface for collision avoidance
 */

#ifndef COLLISIONAVOIDANCE_H_
#define COLLISIONAVOIDANCE_H_

#include "../lanarts_defines.h"

class CombatGameInst;

namespace RVO {
struct RVOSimulator;
}

typedef int simul_id;

class CollisionAvoidance {
public:
	CollisionAvoidance();
	~CollisionAvoidance();

	simul_id add_object(CombatGameInst* inst);
	simul_id add_player_object(CombatGameInst* inst);

	void remove_object(simul_id id);
	void set_preferred_velocity(simul_id id, float vx, float vy);

	void set_position(simul_id, float x, float y);
	void set_maxspeed(simul_id, float maxspeed);
	Posf get_position(simul_id);
	void step();
	void clear();

private:
	RVO::RVOSimulator* simulator;
};

#endif /* COLLISIONAVOIDANCE_H_ */
