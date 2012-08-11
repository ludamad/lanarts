/*
 * PlayerData.cpp:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#include "../lanarts_defines.h"
#include "../util/math_util.h"
#include "../fov/fov.h"

#include "../objects/player/PlayerInst.h"

#include "GameTiles.h"
#include "GameState.h"

#include "PlayerData.h"

void PlayerData::update_fieldsofview(GameState* gs) {
}

void PlayerData::clear() {
	_local_player = GameInstRef();
	_players.clear();
}

void PlayerData::register_player(const std::string& name, PlayerInst* player,
		int net_id) {
	_players.push_back(PlayerDataEntry(name, player, net_id));
	if (player->is_local_player()) {
		_local_player = player;
	}
}

PlayerInst* PlayerData::local_player() {
	LANARTS_ASSERT(
			!_local_player.get_instance() || dynamic_cast<PlayerInst*>(_local_player.get_instance()));
	return (PlayerInst*)_local_player.get_instance();
}

std::vector<PlayerInst*> PlayerData::players_in_level(level_id level) {
	std::vector<PlayerInst*> ret;
	for (int i = 0; i < _players.size(); i++) {
		GameInst* player = _players[i].player_inst.get_instance();
		if (player->current_level == level) {
			LANARTS_ASSERT(!player || dynamic_cast<PlayerInst*>(player));
			ret.push_back((PlayerInst*)player);
		}
	}
	return ret;
}

bool PlayerData::level_has_player(level_id level) {
	for (int i = 0; i < _players.size(); i++) {
		if (_players[i].player_inst->current_level == level) {
			return true;
		}
	}
	return false;
}

void PlayerData::players_gain_xp(GameState* gs, int xp) {
	for (int i = 0; i < _players.size(); i++) {
		PlayerInst* p = (PlayerInst*)_players[i].player_inst.get_instance();
		p->gain_xp(gs, xp);
	}
}

void PlayerData::copy_to(PlayerData& pc) const {
	pc._local_player = _local_player;
	pc._players = _players;
	LANARTS_ASSERT(false /*This has to be fixed to work with object ids!*/);
}

static void write_inst_ref(GameInstRef& ref, GameState* gs,
		SerializeBuffer& serializer) {
	serializer.write_int(ref->id);
	serializer.write_int(ref->current_level);
}
static void read_inst_ref(GameInstRef& ref, GameState* gs,
		SerializeBuffer& serializer) {
	int id, level;
	serializer.read_int(id);
	serializer.read_int(level);
	ref = gs->get_instance(level, id);
}
void PlayerData::serialize(GameState* gs, SerializeBuffer& serializer) {
	write_inst_ref(_local_player, gs, serializer);
	serializer.write_int(_players.size());
	for (int i = 0; i < _players.size(); i++) {

	}
}

void PlayerData::deserialize(GameState* gs, SerializeBuffer & serializer) {

	serializer.write_int(_local_player->id);
	serializer.write_int(_local_player->current_level);
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
