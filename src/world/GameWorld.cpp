/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include "../data/tile_data.h"
#include "../data/class_data.h"

#include "../procedural/levelgen.h"

#include "objects/PlayerInst.h"

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
			local ? get_sprite_by_name("wizard") : get_sprite_by_name("fighter");

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

bool GameWorld::pre_step() {
	if (!game_state->update_iostate())
		return false;

	GameLevelState* current_level = game_state->get_level();

	midstep = true;
	game_state->frame()++;for
(	int i = 0; i < level_states.size(); i++) {
		game_state->set_level(level_states[i]);
		const std::vector<obj_id>& player_ids = game_state->get_level()->pc.player_ids();
		for (int i = 0; i < player_ids.size(); i++) {
			PlayerInst* p = (PlayerInst*)game_state->get_instance(player_ids[i]);
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
//	current_level->steps_left = STEPS_TO_SIMULATE;

	bool inmenu = (game_state->get_level()->level_number == -1);
	midstep = true;

	for (int i = 0; i < level_states.size(); i++) {
		if (level_states[i]->steps_left > 0) {
//			int prehashvalue = game_state->get_level()->inst_set.hash();

//			if (!inmenu && !game_state->net_connection().check_integrity(game_state, prehashvalue)) {
//				printf("Hashes don't match before step, frame %d, level %d\n", game_state->frame(), i);
//			}
			//Set so that all the GameState getters are properly updated
			game_state->set_level(level_states[i]);

//Clone part1:
//			GameLevelState* clone = game_state->level()->clone();
//			GameLevelState* real = game_state->level();
////			level_states[i] = clone;
//            game_state->level() = clone;

			game_state->get_level()->pc.pre_step(game_state);
			game_state->get_level()->mc.pre_step(game_state);
			game_state->get_level()->inst_set.step(game_state);
			game_state->get_level()->tiles.step(game_state);
			game_state->get_level()->steps_left--;
//Clone part2:
////            if (real == current_level) current_level = clone;
//            game_state->level() = real;
//            clone->copy_to(*real);
//            delete clone;

//
//			int posthashvalue =  game_state->level()->inst_set.hash();
//			if (!inmenu &&!game_state->net_connection().check_integrity(game_state, posthashvalue)) {
//				printf("Hashes don't match after step, frame %d, level %d\n", game_state->frame(), i);
//			}
		}
	}
	game_state->set_level(current_level);

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
// 		goto redofirststep;// goto top
	}
}

void GameWorld::regen_level(int roomid) {
	GameLevelState* level = get_level(roomid);
	std::vector<PlayerInst*> player_cache;
	std::vector<obj_id> player_ids = level->pc.player_ids();
	for (int i = 0; i < player_ids.size(); i++) {
		PlayerInst* p = (PlayerInst*)game_state->get_instance(player_ids[i]);
		game_state->remove_instance(p, false); // Remove but do not deallocate
		player_cache.push_back(p);
		p->core_stats().heal_fully();
	}
	level_states[roomid] = NULL;
	GameLevelState* newlevel = get_level(roomid, true, (void**)&player_cache[0],
			player_cache.size());
	if (game_state->get_level() == level) {
		game_state->set_level(newlevel);
		game_state->get_level()->steps_left = 1000;
		GameInst* p = game_state->get_instance(game_state->local_playerid());
		game_state->window_view().sharp_center_on(p->x, p->y);
	}
	delete level;
}

void GameWorld::level_move(int id, int x, int y, int roomid1, int roomid2) {
	//save the level context
	GameLevelState* last = game_state->get_level();
	GameLevelState* state = get_level(roomid1);
	//set the level context
	game_state->set_level(state);

	CombatGameInst* inst = (CombatGameInst*)game_state->get_instance(id);

	Pos hitsqr;
	if (!inst)
		return;
	game_state->remove_instance(inst, false); // Remove but do not deallocate
	inst->last_x = x, inst->last_y = y;
	inst->update_position(x, y);

	game_state->set_level(get_level(roomid2));
	game_state->add_instance(inst);
	game_state->get_level()->steps_left = 1000;

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
		game_state->get_level()->steps_left = 1000;
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
