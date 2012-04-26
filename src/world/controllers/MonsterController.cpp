/*
 * MonsterController.cpp
 *
 *  Created on: Feb 20, 2012
 *      Author: 100397561
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "../GameState.h"
#include "MonsterController.h"
#include "PlayerController.h"

#include "../objects/EnemyInst.h"
#include "../objects/PlayerInst.h"

#include "../../util/draw_util.h"
#include "../../util/collision_util.h"

#include "../../data/tile_data.h"


const int PATHING_RADIUS = 500;
const int HUGE_DISTANCE = 1000000;

MonsterController::MonsterController() {
	targetted = 0;
	must_initialize = true;
	simulator = new RVO::RVOSimulator();
	simulator->setTimeStep(1.0f);
}

MonsterController::~MonsterController() {
	delete simulator;
	resize_paths(0);
}

void MonsterController::register_enemy(GameInst* enemy){
    mids.push_back(enemy->id);
    RVO::Vector2 enemy_position(enemy->x, enemy->y);
    EnemyBehaviour& eb = ((EnemyInst*)enemy)->behaviour();
//    const Vector2& position, float neighborDist,
//                        size_t maxNeighbors, float timeHorizon,
//                        float timeHorizonObst, float radius, float maxSpeed,
//                        const Vector2& velocity = Vector2());
    int simid = simulator->addAgent(enemy_position, 32, 10, 4.0f, 1.0f, 16, eb.speed);
    eb.simulation_id = simid;
}

void towards_highest(PathInfo& path, Pos& p){
	int higest;
	for (int y = -1; y <= + 1; y++){
		for (int x = -1; x <= + 1; x++){
			path.get(p.x +x, p.y + y);
		}
	}

}
//returns true if will be exactly on target
bool move_towards(EnemyInst* e, const Pos& p){
	EnemyBehaviour& eb = e->behaviour();
	float dx = p.x - e->x, dy = p.y - e->y;
	float mag = sqrt(dx*dx+dy*dy);
	if (mag <= eb.speed/2){
		eb.vx = dx;
		eb.vy = dy;
		return true;
	}
	eb.vx = dx/mag*eb.speed/2;
	eb.vy = dy/mag*eb.speed/2;
	return false;
}


void set_preferred_velocity(GameState* gs, RVO::RVOSimulator* sim, EnemyInst* e){
	MTwist& mt = gs->rng();
	EnemyBehaviour& eb = e->behaviour();
  /*
   * Set the preferred velocity to be a vector of unit magnitude (speed) in the
   * direction of the goal.
   */
	RVO::Vector2 goalVector(eb.vx, eb.vy);

	if (RVO::absSq(goalVector) > eb.speed) {
	  goalVector = RVO::normalize(goalVector)*eb.speed;
	}

	/*
	 * Perturb a little to avoid deadlocks due to perfect symmetry.
	 */
	float angle = gs->rng().rand(3600) / 3600.0 * 2.0f * M_PI;
	float dist =  gs->rng().rand(10000) / 10000.0 * 0.0001f;

	goalVector += dist * RVO::Vector2(std::cos(angle), std::sin(angle));

	sim->setAgentPrefVelocity(eb.simulation_id, goalVector);
}

void MonsterController::monster_follow_path(GameState* gs, EnemyInst* e){
	MTwist& mt = gs->rng();
	EnemyBehaviour& eb = e->behaviour();
	if (eb.current_node < eb.path.size()){
		if (move_towards(e, eb.path[eb.current_node]))
			eb.current_node++;

	} else {
		if (mt.rand(6) == 0){
			std::reverse(eb.path.begin(), eb.path.end());
			eb.current_node = 0;
		}
		else
			eb.path.clear();
			eb.current_action = EnemyBehaviour::INACTIVE;
	}
}
void MonsterController::monster_wandering(GameState* gs, EnemyInst* e) {
	GameTiles& tile = gs->tile_grid();
	MTwist& mt = gs->rng();
	EnemyBehaviour& eb = e->behaviour();
	eb.vx = 0, eb.vy = 0;
	bool is_fullpath = true;
	if (eb.path_cooldown > 0){
		eb.path_cooldown--;
		is_fullpath = false;
	}
	int ex = e->x/TILE_SIZE, ey = e->y/TILE_SIZE;

	do {
		int targx, targy;
		do {
			if (!is_fullpath){
				targx = ex+mt.rand(-3, 4);
				targy = ey+mt.rand(-3, 4);
			}else {
				targx = mt.rand(tile.tile_width());
				targy = mt.rand(tile.tile_height());

			}
		} while (game_tile_data[tile.get(targx,targy)].solid);
		eb.current_node = 0;
		eb.path = astarcontext.calculate_AStar_path(gs, ex, ey, targx,targy);
		if (is_fullpath)
			eb.path_cooldown = mt.rand(EnemyBehaviour::RANDOM_WALK_COOLDOWN*2);
		eb.current_action = EnemyBehaviour::FOLLOWING_PATH;
	} while (eb.path.size() <= 1);
}

void MonsterController::set_monster_headings(GameState* gs, std::vector<EnemyOfInterest>& eois) {
	//Use a temporary 'GameView' object to make use of its helper methods
	PlayerController& pc = gs->player_controller();
	for (int i = 0; i < eois.size(); i++) {
		EnemyInst* e = eois[i].e;
		int pind = eois[i].closest_player_index;
		GameInst* p = gs->get_instance(pc.player_ids()[pind]);
		EnemyBehaviour& eb = e->behaviour();

		eb.current_action = EnemyBehaviour::CHASING_PLAYER;
		eb.path.clear();
		eb.action_timeout = 200;
		paths[pind]->interpolated_direction(e->bbox(), eb.speed, eb.vx, eb.vy);

		//Compare position to player object
		double abs = sqrt((e->x-p->x)*(e->x-p->x)+(e->y-p->y)*(e->y-p->y));
		Stats& s = e->stats();

		if (abs < e->radius + p->radius || (s.ranged.canuse && abs < e->radius*2 + p->radius ))
			eb.vx = 0, eb.vy = 0;
		if ( s.melee.canuse && abs < s.melee.range + e->radius ){
			e->attack(gs, p, false);
		} else if ( s.ranged.canuse && abs < s.ranged.range+ 10 ){
			e->attack(gs, p, true);
		}
	}
}

void MonsterController::deregister_enemy(EnemyInst* enemy){
	simulator->setAgentPosition(enemy->behaviour().simulation_id, RVO::Vector2(-TILE_SIZE, -TILE_SIZE));
}
void MonsterController::shift_target(GameState* gs){
	if (!targetted) return;//Should auto-target, it no target no possible targets
	int i, j;
	for (i = 0; i < mids.size(); i++) {
		if (mids[i] == targetted) break;
	}
	GameInst* player = gs->get_instance(gs->local_playerid());

	for (j = i+1; j % mids.size() != i; j++) {
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids[j % mids.size()]);
		if (e == NULL)
			continue;

		bool isvisible = gs->object_visible_test(e, player);
		if (isvisible){
			targetted = e->id;
			return;
		}
	}

}

int MonsterController::find_player_to_target(GameState* gs, EnemyInst* e){
	//Use a 'GameView' object to make use of its helper methods
	GameView view(0, 0, PATHING_RADIUS * 2, PATHING_RADIUS * 2, gs->width(),
			gs->height());

	const std::vector<obj_id>& pids = gs->player_controller().player_ids();
	//Determine which players we are currently in view of
	BBox ebox = e->bbox();
	int mindistsqr = HUGE_DISTANCE;
	int closest_player_index = -1;
	for (int i = 0; i < pids.size(); i++) {
		GameInst* player = gs->get_instance(pids[i]);
		bool isvisible = gs->object_visible_test(e, player);
		if (isvisible) ((PlayerInst*)player)->rest_cooldown() = 150;
		view.sharp_center_on(player->x, player->y);
		bool chasing = e->behaviour().current_action == EnemyBehaviour::CHASING_PLAYER;
		if (view.within_view(ebox) && (chasing || isvisible)) {
			e->behaviour().current_action = EnemyBehaviour::CHASING_PLAYER;

			int dx = e->x - player->x, dy = e->y - player->y;
			int distsqr = dx * dx + dy * dy;
			if (distsqr > 0 /*overflow check*/) {
				if (distsqr < mindistsqr) {
					mindistsqr = distsqr;
					closest_player_index = i;
				}
			}
		}
	}
	return closest_player_index;
}
void MonsterController::process_players(GameState* gs){
	const std::vector<obj_id>& pids = gs->player_controller().player_ids();
	if (pids.size() > player_simids.size()){
		player_simids.resize(pids.size());
		for (int i = 0; i < gs->player_controller().player_ids().size(); i++){
			PlayerInst* p = (PlayerInst*)gs->get_instance(pids[i]);
			player_simids[i] = simulator->addAgent(RVO::Vector2(p->x, p->y), 0, 10, 0.0f, 0.0f, p->radius, 0);
		}
	}

	//Create as many paths as there are players
	paths.resize(pids.size());
	for (int i = 0; i < pids.size(); i++) {
		GameInst* player = gs->get_instance(pids[i]);
		if (paths[i] == NULL)
			paths[i] = new PathInfo;
		paths[i]->calculate_path(gs, player->x, player->y, PATHING_RADIUS);
	}
}


void MonsterController::pre_step(GameState* gs) {

	GameInst* local_player = gs->get_instance(gs->local_playerid());
	std::vector<EnemyOfInterest> eois;

	process_players(gs);

	//Make sure targetted object is alive
	if (targetted && !gs->get_instance(targetted)){
		targetted = 0;
	}

	//Update 'mids' to only hold live objects
	std::vector<obj_id> mids2;
	mids2.reserve(mids.size());
	mids.swap(mids2);
	for (int i = 0; i < mids2.size(); i++) {
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids2[i]);
		if (e == NULL)
			continue;
		e->behaviour().step();

        bool isvisibleToLocal = gs->object_visible_test(e, local_player);
        if (isvisibleToLocal && !targetted) targetted = e->id;
		if (!isvisibleToLocal && targetted == e->id) targetted = 0;

		//Add live instances back to monster id list
		mids.push_back(mids2[i]);

		int closest_player_index = find_player_to_target(gs, e);

		if (closest_player_index == -1 &&
				e->behaviour().current_action == EnemyBehaviour::CHASING_PLAYER)
			e->behaviour().current_action = EnemyBehaviour::INACTIVE;


		if (e->behaviour().current_action == EnemyBehaviour::CHASING_PLAYER)
			eois.push_back(EnemyOfInterest(e, closest_player_index));
		else if (e->behaviour().current_action == EnemyBehaviour::INACTIVE)
			monster_wandering(gs, e);
		else //if (e->behaviour().current_action == EnemyBehaviour::FOLLOWING_PATH)
			monster_follow_path(gs, e);
	}
	set_monster_headings(gs, eois);

	//Handle necessary upkeep of player related information, such as paths to player
	const std::vector<obj_id>& pids = gs->player_controller().player_ids();
	for (int i = 0; i < pids.size(); i++){
		PlayerInst* p = (PlayerInst*)gs->get_instance(gs->player_controller().player_ids()[i]);
		simulator->setAgentPosition(player_simids[i], RVO::Vector2(p->x, p->y));
	}

	for (int i = 0; i < mids.size(); i++) {
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids[i]);
		update_velocity(gs, e);
		set_preferred_velocity(gs, simulator, e);
	}

	simulator->doStep();

	for (int i = 0; i < mids.size(); i++) {
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids[i]);
		update_position(gs, e);
	}
}

void MonsterController::update_velocity(GameState* gs, EnemyInst* e){
//	GameInst* collided = NULL;
//	gs->object_radius_test(e, &collided, 1, &enemy_hit);
	EnemyBehaviour& eb = e->behaviour();
//	if (gs->tile_radius_test(e->x+eb.vx, e->y+eb.vy, e->radius)){
//		eb.vx = -eb.vx;
//		eb.vy = -eb.vy;
//	}


	if (e->stats().hurt_cooldown > 0)
		eb.vx /=2, eb.vy /=2 ;
}
void MonsterController::update_position(GameState* gs, EnemyInst* e){
	EnemyBehaviour& eb = e->behaviour();
	RVO::Vector2 updated = simulator->getAgentPosition(eb.simulation_id);

	float ux = updated.x(), uy = updated.y();
	float dx = ux - e->rx, dy = uy - e->ry;
	float dist = sqrt(dx*dx+dy*dy);

	bool collided = gs->tile_radius_test(round(ux), round(uy), /*e->radius+4*/20);
	int neighbours = simulator->getAgentNumAgentNeighbors(eb.simulation_id);

	if (neighbours > 0 && dist > 0.5f && !collided){
		e->rx = ux;
		e->ry = uy;
	} else {
		float nx = round(e->rx+eb.vx), ny = round(e->ry+eb.vy);
		bool collided = gs->tile_radius_test(nx, ny, e->radius);
		if (collided){
			bool hitsx = gs->tile_radius_test(nx, e->y, e->radius, true, -1);
			bool hitsy = gs->tile_radius_test(e->x, ny, e->radius, true, -1);
			if(hitsy || hitsx || collided){
				if (hitsx) {
					eb.vx = 0;
				}
				if (hitsy) {
					eb.vy = 0;
				}
				if (!hitsy && !hitsx) {
					eb.vx = -eb.vx;
					eb.vy = -eb.vy;
				}
			}
		}
		e->rx += eb.vx;
		e->ry += eb.vy;
		simulator->setAgentPosition(e->behaviour().simulation_id, RVO::Vector2(e->rx, e->ry));
	}

	e->x = (int) round(e->rx); //update based on rounding of true float
	e->y = (int) round(e->ry);
}

void MonsterController::post_draw(GameState* gs){
	GameInst* target = gs->get_instance(targetted);
	if (!target) return;
	glLineWidth(2);
	gl_draw_circle(gs->window_view(), target->x, target->y, target->radius+5, Colour(0,255,0,199), true);
	glLineWidth(1);
}


void MonsterController::clear(){
	paths.clear();
	mids.clear();
}
