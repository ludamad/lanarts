/*
 * PlayerData.cpp:
 *	Holds lists of all players in game, and pointers to their respective PlayerInst's
 */

#include <lcommon/perf_timer.h>

#include "fov/fov.h"
#include "objects/player/PlayerInst.h"
#include <lcommon/math_util.h>

#include "lanarts_defines.h"

#include "GameState.h"
#include "GameTiles.h"

#include "PlayerData.h"

PlayerInst* PlayerDataEntry::player() const {
	GameInst* local = player_inst.get();
	LANARTS_ASSERT(!local || dynamic_cast<PlayerInst*>(local));
	return (PlayerInst*) local;
}

// Used during a game reset
void PlayerData::remove_all_players(GameState* gs) {
	for (int i = 0; i < _players.size(); i++) {
		PlayerInst* p = _players[i].player();
		_players[i].action_queue.clear_actions();
		if (p) {
			int oldlevel = gs->game_world().get_current_level_id();
			gs->game_world().set_current_level(p->current_floor);
			gs->remove_instance(p);
			gs->game_world().set_current_level(oldlevel);
		}
		_players[i].player_inst.clear();
	}
}
void PlayerData::clear() {
	_local_player_idx = -1;
	_players.clear();
}

void PlayerData::register_player(const std::string& name, PlayerInst* player,
		class_id classtype, int net_id) {
	_players.push_back(PlayerDataEntry(name, player, classtype, net_id));
}

PlayerInst* PlayerData::local_player() {
	if (_local_player_idx >= _players.size()) {
		return NULL;
	}
	return local_player_data().player();
}

std::vector<PlayerInst*> PlayerData::players_in_level(level_id level) {
	std::vector<PlayerInst*> ret;
	for (int i = 0; i < _players.size(); i++) {
		GameInst* player = _players[i].player_inst.get();
		if (player && player->current_floor == level) {
			LANARTS_ASSERT(!player || dynamic_cast<PlayerInst*>(player));
			ret.push_back((PlayerInst*) player);
		}
	}
	return ret;
}

bool PlayerData::level_has_player(level_id level) {
	for (int i = 0; i < _players.size(); i++) {
		GameInstRef& p = _players[i].player_inst;
		if (p.get() && p->current_floor == level) {
			return true;
		}
	}
	return false;
}

void PlayerData::copy_to(PlayerData& pc) const {
	pc._local_player_idx = _local_player_idx;
	pc._players = _players;
	LANARTS_ASSERT(false /*This has to be fixed to work with object ids!*/);
}

static void write_inst_ref(GameInstRef& ref, GameState* gs,
		SerializeBuffer& serializer) {
	serializer.write_int(ref->id);
	serializer.write_int(ref->current_floor);
}
static void read_inst_ref(GameInstRef& ref, GameState* gs,
		SerializeBuffer& serializer) {
	int id, level;
	serializer.read_int(id);
	serializer.read_int(level);
	ref = gs->get_instance(level, id);
}

void PlayerData::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_int(_local_player_idx);
	serializer.write_int(_money);
	serializer.write_container(_kill_amounts);
	serializer.write_int(_players.size());
	for (int i = 0; i < _players.size(); i++) {
		write_inst_ref(_players[i].player_inst, gs, serializer);
	}
}

PlayerDataEntry& PlayerData::local_player_data() {
	return _players.at(_local_player_idx);
}

PlayerDataEntry& PlayerData::get_entry_by_netid(int netid) {
	for (int i = 0; i < _players.size(); i++) {
		if (_players[i].net_id == netid) {
			return _players[i];
		}
	}
	/* Shouldn't happen */
	LANARTS_ASSERT(false);
	return _players.at(0);
}

void PlayerData::deserialize(GameState* gs, SerializeBuffer & serializer) {
	serializer.read_int(_local_player_idx);
	serializer.read_int(_money);
	serializer.read_container(_kill_amounts);

	int psize;
	serializer.read_int(psize);
	for (int i = 0; i < psize; i++) {
		PlayerDataEntry& pde = _players.at(i);
		read_inst_ref(pde.player_inst, gs, serializer);
		pde.classtype = pde.player()->class_stats().classid;
		pde.action_queue.clear();
	}

}

void players_gain_xp(GameState* gs, int xp) {
	PlayerData& pd = gs->player_data();
	std::vector<PlayerDataEntry> &players = pd.all_players();

	for (int i = 0; i < players.size(); i++) {
		players[i].player()->gain_xp(gs, xp);
	}
}

int player_get_playernumber(GameState* gs, PlayerInst* p) {
	PlayerData& pd = gs->player_data();
	std::vector<PlayerDataEntry> &players = pd.all_players();

	for (int i = 0; i < players.size(); i++) {
		if (players[i].player() == p) {
			return i;
		}
	}

	LANARTS_ASSERT(false);
	return -1;
}

static bool player_poll_for_actions(GameState* gs, PlayerDataEntry& pde) {
	const int POLL_MS_TIMEOUT = 1 /*millsecond*/;
	GameNetConnection& net = gs->net_connection();
	while (!net.has_incoming_sync()) {
		if (pde.player()->is_local_player()) {
			pde.player()->enqueue_io_actions(gs);
			break;
		} else if (!pde.action_queue.has_actions_for_frame(gs->frame())) {
			if (gs->game_settings().verbose_output) {
				printf("Polling for player %d\n", pde.net_id);
			}
			gs->net_connection().poll_messages(POLL_MS_TIMEOUT);
		} else {

			ActionQueue actions;
			pde.action_queue.extract_actions_for_frame(actions, gs->frame());
			pde.player()->enqueue_actions(actions);
			break;
		}
	}
	return true;
}

bool players_poll_for_actions(GameState* gs) {
	perf_timer_begin("*** NETWORK WAIT ***");

	PlayerData& pd = gs->player_data();
	std::vector<PlayerDataEntry> &players = pd.all_players();

	if (gs->game_settings().verbose_output) {
		printf("Polling for frame %d\n", gs->frame());
	}

	for (int i = 0; i < players.size(); i++) {
		if (!player_poll_for_actions(gs, players[i])) {
			perf_timer_end("*** NETWORK WAIT ***");
			return false;
		}
	}

	perf_timer_end("*** NETWORK WAIT ***");
	return true;
}

void PlayerData::set_local_player_idx(int idx) {
	if (_local_player_idx != -1) {
		PlayerInst* oldlocal = local_player();
		if (oldlocal) {
			oldlocal->set_local_player(false);
		}
	}
	_local_player_idx = idx;
	if (_local_player_idx != -1) {
		PlayerInst* newlocal = local_player();
		if (newlocal) {
			newlocal->set_local_player(true);
		}
	}
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
