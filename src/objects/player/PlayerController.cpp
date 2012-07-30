/*
 * PlayerController.cpp:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#include "../../lanarts_defines.h"
#include "../../util/math_util.h"
#include "../../fov/fov.h"

#include "PlayerInst.h"

#include "../../gamestate/GameTiles.h"
#include "../../gamestate/GameState.h"

#include "PlayerController.h"

void PlayerController::update_fieldsofview(GameState* gs) {
}

void PlayerController::clear() {
	_local_player = GameInstRef();
	_players.clear();
}

void PlayerController::register_player(PlayerInst* player) {
	_players.push_back(player);
	if (player->is_local_player()){
		_local_player = player;
	}
}

PlayerInst* PlayerController::local_player() {
	LANARTS_ASSERT(
			!_local_player.get_instance() || dynamic_cast<PlayerInst*>(_local_player.get_instance()));
	return (PlayerInst*)_local_player.get_instance();
}

std::vector<PlayerInst*> PlayerController::players(level_id level) {
	std::vector<PlayerInst*> ret;
	for (int i = 0; i < _players.size(); i++) {
		GameInst* player = _players[i].get_instance();
		if (player->current_level == level) {
			LANARTS_ASSERT(!player || dynamic_cast<PlayerInst*>(player));
			ret.push_back((PlayerInst*)player);
		}
	}
	return ret;
}

bool PlayerController::level_has_player(level_id level) {
	for (int i = 0; i < _players.size(); i++) {
		if (_players[i]->current_level == level) {
			return true;
		}
	}
	return false;
}

void PlayerController::players_gain_xp(GameState* gs, int xp) {
	for (int i = 0; i < _players.size(); i++) {
		PlayerInst* p = (PlayerInst*)_players[i].get_instance();
		p->gain_xp(gs, xp);
	}
}

void PlayerController::copy_to(PlayerController& pc) const {
	pc._local_player = _local_player;
	pc._players = _players;
	LANARTS_ASSERT(false /*This has to be fixed to work with object ids!*/);
}

//for (int i = 0; i < pids.size(); i++) {
//	PlayerInst* player = (PlayerInst*)gs->get_instance(pids[i]);
//	int sx = player->last_x * VISION_SUBSQRS / TILE_SIZE;
//	int sy = player->last_y * VISION_SUBSQRS / TILE_SIZE;
//	player->field_of_view().calculate(gs, sx, sy);
//}
//}
//void PlayerController::pre_step(GameState* gs) {
//update_fieldsofview(gs);
//}
//
//void PlayerController::players_gain_xp(GameState* gs, int xp) {
//for (int i = 0; i < pids.size(); i++) {
//	PlayerInst* p = (PlayerInst*)gs->get_instance(pids[i]);
//	p->gain_xp(gs, xp);
//}
//}
//
//void PlayerController::copy_to(PlayerController& pc) const {
//pc.local_player = this->local_player;
//pc.pids.clear();
//
//for (int i = 0; i < this->pids.size(); i++) {
//	pc.pids.push_back(this->pids[i]);
//}
//}
//
//void PlayerController::register_player(obj_id player, bool islocal) {
//if (islocal)
//	local_player = player;
//pids.push_back(player);
//}
//
//void PlayerController::deregister_player(obj_id player) {
//if (player == local_player) {
//	local_player = 0;
//}
//int i;
//for (i = 0; i < pids.size() && pids[i] != player; i++) {
//	//find 'i' such that pids[i] == player
//}
//if (i == pids.size()) {
//	printf("Didn't find player in list\n");
//	return;
//}
//pids.erase(pids.begin() + i);
//printf("Deregistering '%d'\n", player);
//
//fflush(stdout);
