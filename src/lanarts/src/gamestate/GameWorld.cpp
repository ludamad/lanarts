/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include <typeinfo>

#include "display/TileEntry.h"
#include "levelgen/levelgen.h"

#include "objects/enemy/EnemyInst.h"

#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"
#include "objects/GameInstRef.h"
#include "stats/class_data.h"

#include "GameLevelState.h"
#include "GameState.h"
#include "GameWorld.h"

GameWorld::GameWorld(GameState* gs) :
		gs(gs), next_room_id(-1) {
	midstep = false;
	lvl = NULL;
}

GameWorld::~GameWorld() {
	reset();
}

void GameWorld::generate_room(GameLevelState* level) {

}

level_id GameWorld::get_current_level_id() {
	return gs->get_level()->id();
}

void GameWorld::place_inst(GeneratedLevel& genlevel, GameInst* inst) {
	GameTiles& tiles = gs->tiles();
	Pos epos;
	do {
		epos = generate_location(gs->rng(), genlevel);
	} while (!genlevel.at(epos).passable || genlevel.at(epos).has_instance);

	genlevel.at(epos).has_instance = true;
	Pos spawn_pos = genlevel.get_world_coordinate(epos);

	inst->last_x = spawn_pos.x;
	inst->last_y = spawn_pos.y;
	inst->update_position(spawn_pos.x, spawn_pos.y);

	gs->add_instance(inst);
}

void GameWorld::serialize(SerializeBuffer& serializer) {
	_enemies_seen.serialize(serializer);

	serializer.write_int(level_states.size());
	GameLevelState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		GameLevelState* lvl = level_states[i];
		serializer.write(lvl->width());
		serializer.write(lvl->height());
		gs->set_level(lvl);
		lvl->serialize(gs, serializer);
	}

	gs->set_level(original);
}

void GameWorld::deserialize(SerializeBuffer& serializer) {
	_enemies_seen.deserialize(serializer);

	int nlevels;
	serializer.read_int(nlevels);
	for (int i = nlevels; i < level_states.size(); i++) {
		delete level_states.at(i);
	}
	level_states.resize(nlevels, NULL);

	GameLevelState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		int width, height;
		serializer.read(width);
		serializer.read(height);
		if (level_states[i] == NULL) {
			level_states[i] = new GameLevelState(i, width, height, false,
					false);
		}
		gs->set_level(level_states[i]);
		level_states[i]->deserialize(gs, serializer);
	}

	gs->set_level(original);
	midstep = false;
}

void GameWorld::spawn_players(GeneratedLevel& genlevel, void** player_instances,
		size_t nplayers) {
	if (!player_instances) {
		bool flocal = (gs->game_settings().conntype == GameSettings::CLIENT);
		GameSettings& settings = gs->game_settings();
		GameNetConnection& netconn = gs->net_connection();
		int myclassn = gs->game_settings().classtype;

		for (int i = 0; i < gs->player_data().all_players().size(); i++) {
			PlayerDataEntry& pde = gs->player_data().all_players()[i];
			bool islocal = &pde == &gs->player_data().local_player_data();
			ClassType& c = game_class_data.at(pde.classtype);
			int spriteidx = gs->rng().rand(c.sprites.size());

			if (pde.player_inst.empty()) {
				pde.player_inst = new PlayerInst(c.starting_stats,
						c.sprites[spriteidx], 0, 0, islocal);
			}
			printf("Spawning for player %d: %s\n", i,
					islocal ? "local player" : "network player");
			place_inst(genlevel, pde.player_inst.get_instance());
		}
	} else {
		for (int i = 0; i < nplayers; i++) {
			place_inst(genlevel, (GameInst*)player_instances[i]);
		}
	}
}
GameLevelState* GameWorld::get_level(int roomid, bool spawnplayer,
		void** player_instances, size_t nplayers) {
	GameLevelState* currlvl = gs->get_level(); // Save level context

	if (roomid >= level_states.size()) {
		level_states.resize(roomid + 1, NULL);
	}
	if (!level_states[roomid]) {
		GeneratedLevel genlevel;
		GameLevelState* newlvl = generate_level(roomid, gs->rng(), genlevel,
				gs);
		level_states[roomid] = newlvl;
		gs->set_level(newlvl);

		if (spawnplayer) {
			spawn_players(genlevel, player_instances, nplayers);
		}
	}

	gs->set_level(currlvl); // Restore level context
	return level_states[roomid];
}

static bool check_level_is_in_sync(GameState* gs, GameLevelState* level) {
	if (level->id() == -1 || !gs->player_data().level_has_player(level->id())
			|| !gs->net_connection().connection()) {
		return true;
	}

	int hash_value = level->game_inst_set().hash();

//	printf("NETWORK: Checking integrity hash=0x%X\n", prehashvalue);
//	fflush(stdout);

	//TODO: net redo
//	if (!gs->net_connection().check_integrity(gs, hash_value)) {
//		std::vector<GameInst*> instances = level->inst_set.to_vector();
//		for (int i = 0; i < instances.size(); i++) {
//			if (!gs->net_connection().check_integrity(gs,
//					instances[i]->integrity_hash())) {
//				GameInst* inst = instances[i];
//				if (!dynamic_cast<AnimatedInst*>(inst)) {
//					const char* type = typeid(*inst).name();
//					printf("Hashes don't match for instance id=%d, type=%s!\n",
//							inst->id, type);
//					std::vector<Pos> theirs;
//					gs->net_connection().send_and_sync(Pos(inst->x, inst->y),
//							theirs);
//					Pos* theirinst = &theirs[0];
//					EnemyInst* e;
//					if ((e = dynamic_cast<EnemyInst*>(inst))) {
//						Pos vpos(e->vx * 1000, e->vy * 1000);
//						std::vector<Pos> theirs;
//						gs->net_connection().send_and_sync(vpos, theirs);
//						Pos vpos2 = theirs[0];
//					}
//				}
//
//			}
//		}
//		printf("Hashes don't match before step, frame %d, level %d\n",
//				gs->frame(), level->levelid);
//		return false;
//	}
	return true;
}
bool GameWorld::pre_step() {
	if (!gs->update_iostate())
		return false;

	GameLevelState* current_level = gs->get_level();

	midstep = true;

	/* Check level sync states */
	if (gs->game_settings().network_debug_mode) {
		for (int i = 0; i < level_states.size(); i++) {
			check_level_is_in_sync(gs, level_states[i]);
		}
	}

	/* Queue actions for local player */
	/* This will result in a network send of the player's actions */
	PlayerData& pd = gs->player_data();
	pd.local_player()->enqueue_io_actions(gs);

	gs->set_level(current_level);
	midstep = false;
	return true;
}
void GameWorld::step() {
	redofirststep: //I used a goto dont kill mes

	GameLevelState* current_level = gs->get_level();

	midstep = true;

	/* Queue all actions for players */
	/* This will result in a network poll for other players actions */
	players_poll_for_actions(gs);

	for (int i = 0; i < level_states.size(); i++) {
		GameLevelState* level = level_states[i];
		level->step(gs);
	}

	midstep
	= false;
	if (next_room_id == -2) {
		reset(0);
		next_room_id = 0;
	}
	if (next_room_id != -1) {
		set_current_level(next_room_id);
		gs->view().sharp_center_on(gs->local_player()->pos());
		next_room_id = -1;
	}
}

void GameWorld::regen_level(int roomid) {
	GameLevelState* level = get_level(roomid);
	std::vector<PlayerInst*> players = player_data().players_in_level(roomid);

	/* Take all players out of level*/
	for (int i = 0; i < players.size(); i++) {
		// Get and retain player
		players[i]->core_stats().heal_fully();

		// Remove from current level
		gs->remove_instance(players[i]);
	}

	level_states[roomid] = NULL;

	GameLevelState* newlevel = get_level(roomid, true, (void**)&players[0],
			players.size());

	if (gs->get_level() == level) {
		gs->game_hud().override_sidebar_contents(NULL);
		gs->set_level(newlevel);
		GameInst* p = gs->local_player();
		gs->view().sharp_center_on(p->x, p->y);
	}

	//Delete existing level
	delete level;
}

void GameWorld::level_move(int id, int x, int y, int roomid1, int roomid2) {
	//save the level context
	GameLevelState* last = gs->get_level();
	GameLevelState* state = get_level(roomid1);
	//set the level context
	gs->set_level(state);

	//Keep object alive during move
	GameInstRef gref(gs->get_instance(id));
	GameInst* inst = gref.get_instance();

	if (!gref.get_instance()) {
		return;
	}

	gs->remove_instance(inst);
	gref->last_x = x, gref->last_y = y;
	gref->update_position(x, y);

	gs->set_level(get_level(roomid2));
	gs->add_instance(inst);

	PlayerInst* p;
	if ((p = dynamic_cast<PlayerInst*>(inst))) {
		p->update_field_of_view(gs);
		if (p->is_local_player()) {
			set_current_level_lazy(roomid2);
		}
	}

	//restore the level context
	gs->set_level(last);

}

void GameWorld::set_current_level(int roomid) {
	if (midstep)
		set_current_level_lazy(roomid);
	else
		gs->set_level(get_level(roomid));
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
		player_data().remove_all_players(gs);
		level_states.resize(keep);
		gs->game_hud().override_sidebar_contents(NULL);
		gs->set_level(get_level(keep, true /*spawn player*/));
		gs->game_chat().clear();
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
