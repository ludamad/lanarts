/*
 * CollisionAvoidance.h:
 *  A stable interface for collision avoidance
 */

#include <rvo2/RVO.h>

#include "../world/objects/CombatGameInst.h"

#include "CollisionAvoidance.h"

CollisionAvoidance::CollisionAvoidance() {
	simulator = new RVO::RVOSimulator();
	simulator->setTimeStep(1.0f);
}

CollisionAvoidance::~CollisionAvoidance() {
	delete simulator;
}

simul_id CollisionAvoidance::add_to_simulation(CombatGameInst* inst) {
	RVO::Vector2 enemy_position(inst->x, inst->y);
	EffectiveStats& estats = inst->effective_stats();

	return simulator->addAgent(enemy_position, 32, 10, 4.0f, 1.0f, 16,
			estats.movespeed);
}

void CollisionAvoidance::remove_from_simulation(simul_id id) {
	//NO-OP for rvo ??
}

void CollisionAvoidance::set_preferred_velocity(simul_id id, float vx,
		float vy) {
	simulator->setAgentPrefVelocity(id, RVO::Vector2(vx, vy));
}

void CollisionAvoidance::set_position_and_maxspeed(simul_id id, float x,
		float y, float maxspeed) {
	simulator->setAgentPosition(id, RVO::Vector2(x, y));
	simulator->setAgentMaxSpeed(id, maxspeed);
}

void CollisionAvoidance::step() {
	simulator->doStep();
}

