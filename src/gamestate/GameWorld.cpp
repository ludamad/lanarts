/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include <typeinfo>

#include "../display/tile_data.h"
#include "../stats/class_data.h"

#include "../levelgen/levelgen.h"

#include "../objects/GameInstRef.h"
#include "../objects/player/PlayerInst.h"
#include "../objects/enemy/EnemyInst.h"
#include "../objects/AnimatedInst.h"

#include "GameState.h"
#include "GameLevelState.h"
#include "GameWorld.h"

GameWorld::GameWorld(GameState* gs) :
		game_state(gs), next_room_id(-1) {
	midstep = false;
	lvl = NULL;
}

GameWorld::~GameWorld() {
	reset();
}

void GameWorld::generate_room(GameLevelState* level) {

}

level_id GameWorld::get_current_level_id() {
	return game_state->get_level()->levelid;
}

void GameWorld::spawn_player(GeneratedLevel& genlevel, bool local, int classn,
		PlayerInst* inst) {
	GameTiles& tiles = game_state->tile_grid();
	ClassType& c = game_class_data.at(classn);
	Pos epos;
	do {
		epos = generate_location(game_state->rng(), genlevel);
	} while (!genlevel.at(epos).passable || genlevel.at(epos).has_instance);

	genlevel.at(epos).has_instance = true;
	Pos spawn_pos = genlevel.get_world_coordinate(epos);

	if (!inst) {
		inst = new PlayerInst(c.starting_stats, spawn_pos.x, spawn_pos.y,
				local);
		player_controller().register_player(inst);
	}
	inst->last_x = spawn_pos.x;
	inst->last_y = spawn_pos.y;
	inst->update_position(spawn_pos.x, spawn_pos.y);

	game_state->add_instance(inst);
}

void GameWorld::spawn_players(GeneratedLevel& genlevel, void** player_instances,
		size_t nplayers) {
	if (!player_instances) {
		bool flocal = (game_state->game_settings().conntype
				== GameSettings::CLIENT);
		GameSettings& settings = game_state->game_settings();
		GameNetConnection& netconn = game_state->net_connection();
		int myclassn = game_state->game_settings().classn;

		static std::vector<int> theirclasses;
		if (theirclasses.empty() && netconn.get_connection()) {
			NetPacket classpacket;
			classpacket.add_int(myclassn);
			classpacket.encode_header();
			std::vector<NetPacket> others_classes;
			netconn.send_and_sync(classpacket, others_classes, true);
			for (int i = 0; i < others_classes.size(); i++) {
				theirclasses.push_back(others_classes[i].get_int());
			}
		}

		if (theirclasses.empty()) {
			spawn_player(genlevel, true, myclassn);
		} else {
			spawn_player(genlevel, flocal, flocal ? myclassn : theirclasses[0]);
			spawn_player(genlevel, !flocal,
					!flocal ? myclassn : theirclasses[0]);
		}
	} else {
		for (int i = 0; i < nplayers; i++) {
			spawn_player(genlevel, false, 0, (PlayerInst*)player_instances[i]);
		}
	}
}
GameLevelState* GameWorld::get_level(int roomid, bool spawnplayer,
		void** player_instances, size_t nplayers) {
	GameLevelState* currlvl = game_state->get_level(); // Save level context

	if (roomid >= level_states.size()) {
		level_states.resize(roomid + 1, NULL);
	}
	if (!level_states[roomid]) {
		GeneratedLevel genlevel;
		GameLevelState* newlvl = generate_level(roomid, game_state->rng(),
				genlevel, game_state);
		level_states[roomid] = newlvl;
		game_state->set_level(newlvl);

		if (spawnplayer) {
			spawn_players(genlevel, player_instances, nplayers);
		}
	}

	game_state->set_level(currlvl); // Restore level context
	return level_states[roomid];
}

static bool check_level_is_in_sync(GameState* gs, GameLevelState* level) {
	if (level->levelid == -1
			|| !gs->player_controller().level_has_player(level->levelid)
			|| !gs->net_connection().get_connection()) {
		return true;
	}

	int hash_value = level->inst_set.hash();

//	printf("NETWORK: Checking integrity hash=0x%X\n", prehashvalue);
//	fflush(stdout);
	if (!gs->net_connection().check_integrity(gs, hash_value)) {
		std::vector<GameInst*> instances = level->inst_set.to_vector();
		for (int i = 0; i < instances.size(); i++) {
			if (!gs->net_connection().check_integrity(gs,
					instances[i]->integrity_hash())) {
				GameInst* inst = instances[i];
				if (!dynamic_cast<AnimatedInst*>(inst)) {
					const char* type = typeid(*inst).name();
					printf("Hashes don't match for instance id=%d, type=%s!\n",
							inst->id, type);
					std::vector<Pos> theirs;
					gs->net_connection().send_and_sync(Pos(inst->x, inst->y),
							theirs);
					Pos* theirinst = &theirs[0];
					EnemyInst* e;
					if ((e = dynamic_cast<EnemyInst*>(inst))) {
						Pos vpos(e->vx * 1000, e->vy * 1000);
						std::vector<Pos> theirs;
						gs->net_connection().send_and_sync(vpos, theirs);
						Pos vpos2 = theirs[0];
					}
				}

			}
		}
		printf("Hashes don't match before step, frame %d, level %d\n",
				gs->frame(), level->levelid);
		return false;
	}
	return true;
}
bool GameWorld::pre_step() {
	if (!game_state->update_iostate())
		return false;

	GameLevelState* current_level = game_state->get_level();

	midstep = true;

	/* Check level sync states */
	if (game_state->game_settings().network_debug_mode) {
		for (int i = 0; i < level_states.size(); i++) {
			check_level_is_in_sync(game_state, level_states[i]);
		}
	}

	/* Queue actions for each player */
	for (int i = 0; i < level_states.size(); i++) {
		game_state->set_level(level_states[i]);
		std::vector<PlayerInst*> players = game_state->players_in_level();
		for (int i = 0; i < players.size(); i++) {
			PlayerInst* p = players[i];
			p->queue_io_actions(game_state);
			p->performed_actions_for_step() = false;
		}
	}
	game_state->set_level(current_level);
	midstep = false;
	return true;
}
static void update_player_fields_of_view(GameState* gs) {
	std::vector<PlayerInst*> players = gs->players_in_level();
	for (int i = 0; i < players.size(); i++) {
		players[i]->update_field_of_view(gs);
	}
}
void GameWorld::step() {
	redofirststep: //I used a goto dont kill me

	const int STEPS_TO_SIMULATE = 1000;
	GameLevelState* current_level = game_state->get_level();

	midstep = true;

	for (int i = 0; i < level_states.size(); i++) {
		GameLevelState* level = level_states[i];
		bool has_player_in_level = player_controller().level_has_player(
				level->levelid);

		if (has_player_in_level) {
			level->steps_left = STEPS_TO_SIMULATE;
		}

		if (level->steps_left > 0) {
			//Set so that all the GameState getters are properly updated
			game_state->set_level(level);

//Clone part1:
//			GameLevelState* clone = game_state->level()->clone();
//			GameLevelState* real = game_state->level();
////			level_states[i] = clone;
//            game_state->level() = clone;
			update_player_fields_of_view(game_state);
			level->mc.pre_step(game_state);
			level->inst_set.step(game_state);
			level->tiles.step(game_state);
			level->steps_left--;
//Clone part2:
////            if (real == current_level) current_level = clone;
//            game_state->level() = real;
//            clone->copy_to(*real);
//            delete clone;

//
		}
	}
	game_state->set_level(current_level);
	game_state->frame()++;

	midstep
	= false;
	if (next_room_id == -2) {
		reset(0);
		next_room_id = 0;
	}
	if (next_room_id != -1) {
		set_current_level(next_room_id);
		game_state->view().sharp_center_on(game_state->local_player()->pos());
		next_room_id = -1;
	}
}

void GameWorld::regen_level(int roomid) {
	GameLevelState* level = get_level(roomid);
	std::vector<PlayerInst*> players = player_controller().players_in_level(
			roomid);

	/* Take all players out of level*/
	for (int i = 0; i < players.size(); i++) {
		// Get and retain player
		players[i]->core_stats().heal_fully();

		// Remove from current level
		game_state->remove_instance(players[i]);
	}

	level_states[roomid] = NULL;

	GameLevelState* newlevel = get_level(roomid, true, (void**)&players[0],
			players.size());

	if (game_state->get_level() == level) {
		game_state->game_hud().override_sidebar_contents(NULL);
		game_state->set_level(newlevel);
		GameInst* p = game_state->local_player();
		game_state->view().sharp_center_on(p->x, p->y);
	}

	//Delete existing level
	delete level;
}

void GameWorld::level_move(int id, int x, int y, int roomid1, int roomid2) {
	//save the level context
	GameLevelState* last = game_state->get_level();
	GameLevelState* state = get_level(roomid1);
	//set the level context
	game_state->set_level(state);

	//Keep object alive during move
	GameInstRef gref(game_state->get_instance(id));
	GameInst* inst = gref.get_instance();

	if (!gref.get_instance()) {
		return;
	}

	game_state->remove_instance(inst);
	gref->last_x = x, gref->last_y = y;
	gref->update_position(x, y);

	game_state->set_level(get_level(roomid2));
	game_state->add_instance(inst);

	PlayerInst* p;
	if ((p = dynamic_cast<PlayerInst*>(inst))) {
		p->update_field_of_view(game_state);
		if (p->is_local_player()) {
			set_current_level_lazy(roomid2);
		}
	}

	//restore the level context
	game_state->set_level(last);

}

void GameWorld::set_current_level(int roomid) {
	if (midstep)
		set_current_level_lazy(roomid);
	else
		game_state->set_level(get_level(roomid));
}
void GameWorld::set_current_level_lazy(int roomid) {
	next_room_id = roomid;
}

void GameWorld::reset(int keep) {
	std::vector<GameLevelState*> delete_list;
	if (midstep) {
		next_room_id = -2;
	} else {
		for (int i = keep; i < level_states.size(); i++) {
			delete_list.push_back(level_states[i]);
		}
		level_states.resize(keep);
		player_controller().clear();
		game_state->game_hud().override_sidebar_contents(NULL);
		game_state->set_level(get_level(keep, true /*spawn player*/));
		game_state->game_chat().clear();
	}
	for (int i = 0; i < delete_list.size(); i++) {
		delete delete_list[i];
	}
}

void GameWorld::connect_entrance_to_exit(int roomid1, int roomid2) {
	GameLevelState* l1 = get_level(roomid1);
	GameLevelState* l2 = get_level(roomid2);
	LANARTS_ASSERT(l2->exits.size() == l1->entrances.size());
	for (int i = 0; i < l2->exits.size(); i++) {
		l2->exits[i].exitsqr = l1->entrances[i].entrancesqr;
		l1->entrances[i].exitsqr = l2->exits[i].entrancesqr;
	}
}
