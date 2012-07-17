/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include <typeinfo>

#include "../data/tile_data.h"
#include "../data/class_data.h"

#include "../procedural/levelgen.h"

#include "objects/GameInstRef.h"
#include "objects/PlayerInst.h"
#include "objects/EnemyInst.h"
#include "utility_objects/AnimatedInst.h"

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

int GameWorld::get_current_level_id() {
	return game_state->get_level()->roomid;
}

void GameWorld::spawn_player(GeneratedLevel& genlevel, bool local, int classn,
		PlayerInst* inst) {
	GameTiles& tiles = game_state->tile_grid();
	ClassType& c = game_class_data.at(classn);
	Pos epos;
	do {
		epos = generate_location(game_state->rng(), genlevel);
	} while (!genlevel.at(epos).passable || genlevel.at(epos).has_instance);

	int start_x = (tiles.tile_width() - genlevel.width()) / 2;
	int start_y = (tiles.tile_height() - genlevel.height()) / 2;

	genlevel.at(epos).has_instance = true;
	int px = (epos.x + start_x) * TILE_SIZE + TILE_SIZE / 2;
	int py = (epos.y + start_y) * TILE_SIZE + TILE_SIZE / 2;

	sprite_id sprite =
			local ? get_sprite_by_name("wizard") : get_sprite_by_name(
							"fighter");

	if (!inst) {
		inst = new PlayerInst(c.starting_stats, sprite, px, py, local);
	}
	inst->last_x = px;
	inst->last_y = py;
	inst->update_position(px, py);

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
	if (level->level_number == -1 || !level->pc.has_player()
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
				gs->frame(), level->level_number);
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
		const std::vector<obj_id>& player_ids =
				game_state->get_level()->pc.player_ids();
		for (int i = 0; i < player_ids.size(); i++) {
			PlayerInst* p = (PlayerInst*)game_state->get_instance(
					player_ids[i]);
			p->queue_io_actions(game_state);
			p->performed_actions_for_step() = false;
		}
	}
	game_state->set_level(current_level);
	midstep = false;
	return true;
}
void GameWorld::step() {
	redofirststep: //I used a goto dont kill me

	const int STEPS_TO_SIMULATE = 1000;
	GameLevelState* current_level = game_state->get_level();

	midstep = true;

	for (int i = 0; i < level_states.size(); i++) {
		GameLevelState* level = level_states[i];
		bool has_player_in_level = level->pc.has_player();

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

			level->pc.pre_step(game_state);
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

	midstep = false;
	if (next_room_id == -2) {
		reset(0);
		next_room_id = 0;
	}
	if (next_room_id != -1) {
		set_current_level(next_room_id);
		GameInst* g = game_state->get_instance(
				game_state->player_controller().local_playerid());
		game_state->window_view().sharp_center_on(g->x, g->y);
		next_room_id = -1;

		game_state->get_level()->pc.update_fieldsofview(game_state);
	}
}

void GameWorld::regen_level(int roomid) {
	GameLevelState* level = get_level(roomid);
	std::vector<GameInstRef> player_cache;
	std::vector<obj_id> player_ids = level->pc.player_ids();

	/* Take all players out of level*/
	for (int i = 0; i < player_ids.size(); i++) {
		// Get and retain player
		GameInst* p = game_state->get_instance(player_ids[i]);
		player_cache.push_back(p);
		((PlayerInst*)p)->core_stats().heal_fully();

		// Remove from current level
		game_state->remove_instance(p);
	}

	level_states[roomid] = NULL;

	GameLevelState* newlevel = get_level(roomid, true, (void**)&player_cache[0],
			player_cache.size());

	if (game_state->get_level() == level) {
		game_state->set_level(newlevel);
		GameInst* p = game_state->get_instance(game_state->local_playerid());
		game_state->window_view().sharp_center_on(p->x, p->y);
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
	//restore the level context
	game_state->set_level(last);

	PlayerInst* p;
	if ((p = dynamic_cast<PlayerInst*>(inst)) && p->is_local_player()) {
		set_current_level_lazy(roomid2);
	}

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
