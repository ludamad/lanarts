/*
 * CollisionAvoidance.h:
 *  A stable interface for collision avoidance
 */

#include <lcommon/PerfTimer.h>
#include <rvo2/RVO.h>

#include "objects/CombatGameInst.h"

#include "CollisionAvoidance.h"

CollisionAvoidance::CollisionAvoidance() {
	simulator = new RVO::RVOSimulator();
	simulator->setTimeStep(1.0f);
}

CollisionAvoidance::~CollisionAvoidance() {
	delete simulator;
}

simul_id CollisionAvoidance::add_active_object(const Pos& pos, int radius,
		float movespeed) {
	const int MAX_NEIGHBOURS = 10;
	const float TIME_HORIZON = 8.0f, TIME_STEP = 1.0f;
	const float neighbour_dist = radius * 2;

	RVO::Vector2 position(pos.x, pos.y);

	return simulator->addAgent(position, neighbour_dist, MAX_NEIGHBOURS,
			TIME_HORIZON, TIME_STEP, radius, movespeed);
}

simul_id CollisionAvoidance::add_passive_object(const Pos& pos, int radius) {
	const int MAX_NEIGHBOURS = 10;
	const float TIME_HORIZON = 0.0f, TIME_STEP = 1.0f;
	const float NEIGHBOUR_DIST = 0.0f;
	const float MOVESPEED = 0.0f;

	RVO::Vector2 position(pos.x, pos.y);

	return simulator->addAgent(position, NEIGHBOUR_DIST, MAX_NEIGHBOURS,
			TIME_HORIZON, TIME_STEP, radius, MOVESPEED);
}

simul_id CollisionAvoidance::add_object(CombatGameInst* inst) {
	RVO::Vector2 enemy_position(inst->x, inst->y);
	EffectiveStats& estats = inst->effective_stats();

	return simulator->addAgent(enemy_position, inst->target_radius * 2, 10,
			8.0f, 1.0f, inst->target_radius, estats.movespeed);
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
	perf_timer_begin(FUNCNAME);
	simulator->doStep();
	perf_timer_end(FUNCNAME);
}

PosF CollisionAvoidance::get_position(simul_id id) {
	RVO::Vector2 pos = simulator->getAgentPosition(id);
	return PosF(pos.x(), pos.y());
}

void CollisionAvoidance::clear() {
	simulator = new RVO::RVOSimulator();
	simulator->setTimeStep(1.0f);
}

void avoid_object_collisions(CollisionAvoidance& colavoid,
		const std::vector<CombatGameInst*>& objects) {
	for (int i = 0; i < objects.size(); i++) {

	}
}
