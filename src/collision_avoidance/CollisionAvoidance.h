/*
 * CollisionAvoidance.h:
 *  A stable interface for collision avoidance
 */

#ifndef COLLISIONAVOIDANCE_H_
#define COLLISIONAVOIDANCE_H_

class CombatGameInst;

namespace RVO {
struct RVOSimulator;
}

typedef int simul_id;

class CollisionAvoidance {
public:
	CollisionAvoidance();
	~CollisionAvoidance();

	simul_id add_to_simulation(CombatGameInst* inst);
	void remove_from_simulation(simul_id id);
	void set_preferred_velocity(simul_id id, float vx, float vy);
	void set_position_and_maxspeed(simul_id, float x, float y, float maxspeed);

	void step();

private:
	RVO::RVOSimulator* simulator;
};

#endif /* COLLISIONAVOIDANCE_H_ */
