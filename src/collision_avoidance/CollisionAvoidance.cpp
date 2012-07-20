/*
 * CollisionAvoidance.h:
 *  A stable interface for collision avoidance
 */

#include <rvo2/RVO.h>

#include "../objects/CombatGameInst.h"

#include "CollisionAvoidance.h"

CollisionAvoidance::CollisionAvoidance() {
	simulator = new RVO::RVOSimulator();
	simulator->setTimeStep(1.0f);
}

CollisionAvoidance::~CollisionAvoidance() {
	delete simulator;
}

simul_id CollisionAvoidance::add_object(CombatGameInst* inst) {
	RVO::Vector2 enemy_position(inst->x, inst->y);
	EffectiveStats& estats = inst->effective_stats();

	return simulator->addAgent(enemy_position, inst->target_radius*2, 10, 8.0f, 1.0f, inst->target_radius,
			estats.movespeed);
}
simul_id CollisionAvoidance::add_player_object(CombatGameInst* inst) {
	RVO::Vector2 enemy_position(inst->x, inst->y);
	EffectiveStats& estats = inst->effective_stats();

	return simulator->addAgent(enemy_position, 0, 10, 0.0f, 0.0f, inst->radius,
			0);
}

void CollisionAvoidance::remove_object(simul_id id) {
	simulator->removeAgent(id);
}

void CollisionAvoidance::set_preferred_velocity(simul_id id, float vx,
		float vy) {
	simulator->setAgentPrefVelocity(id, RVO::Vector2(vx, vy));
}

void CollisionAvoidance::set_position(simul_id id, float x, float y) {
	simulator->setAgentPosition(id, RVO::Vector2(x, y));
}

void CollisionAvoidance::set_maxspeed(simul_id id, float maxspeed) {
	simulator->setAgentMaxSpeed(id, maxspeed);
}

void CollisionAvoidance::step() {
	simulator->doStep();
}

Posf CollisionAvoidance::get_position(simul_id id) {
	RVO::Vector2 pos = simulator->getAgentPosition(id);
	return Posf(pos.x(), pos.y());
}

void CollisionAvoidance::clear() {
	simulator = new RVO::RVOSimulator();
	simulator->setTimeStep(1.0f);
}

