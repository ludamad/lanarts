/*
 * PlayerController.cpp:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#include "../../util/math_util.h"
#include "../../fov/fov.h"

#include "../objects/PlayerInst.h"

#include "../GameTiles.h"
#include "../GameState.h"

#include "PlayerController.h"

PlayerController::PlayerController() :
		local_player(0) {
}

PlayerController::~PlayerController() {
}

void PlayerController::update_fieldsofview(GameState* gs) {
	for (int i = 0; i < pids.size(); i++) {
		PlayerInst* player = (PlayerInst*)gs->get_instance(pids[i]);
		int sx = player->last_x * VISION_SUBSQRS / TILE_SIZE;
		int sy = player->last_y * VISION_SUBSQRS / TILE_SIZE;
		player->field_of_view().calculate(gs, sx, sy);
	}
}
void PlayerController::pre_step(GameState* gs) {
	update_fieldsofview(gs);
}

void PlayerController::players_gain_xp(GameState* gs, int xp) {
	for (int i = 0; i < pids.size(); i++) {
		PlayerInst* p = (PlayerInst*)gs->get_instance(pids[i]);
		p->gain_xp(gs, xp);
	}
}

void PlayerController::copy_to(PlayerController& pc) const {
	pc.local_player = this->local_player;
	pc.pids.clear();

	for (int i = 0; i < this->pids.size(); i++) {
		pc.pids.push_back(this->pids[i]);
	}
}

void PlayerController::register_player(obj_id player, bool islocal) {
	if (islocal)
		local_player = player;
	pids.push_back(player);
}

void PlayerController::deregister_player(obj_id player) {
	if (player == local_player) {
		local_player = 0;
	}
	int i;
	for (i = 0; i < pids.size() && pids[i] != player; i++) {
		//find 'i' such that pids[i] == player
	}
	if (i == pids.size()) {
		printf("Didn't find player in list\n");
		return;
	}
	pids.erase(pids.begin() + i);
	printf("Deregistering '%d'\n", player);

	fflush(stdout);
}

void PlayerController::clear() {
	pids.clear();
}

