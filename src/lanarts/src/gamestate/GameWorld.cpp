/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include <typeinfo>

#include "draw/TileEntry.h"
#include "dungeon_generation/levelgen.h"

#include "objects/enemy/EnemyInst.h"

#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"
#include "objects/GameInstRef.h"
#include "stats/ClassEntry.h"

#include "GameLevelState.h"
#include "GameState.h"
#include "GameWorld.h"
#include "ScoreBoard.h"

GameWorld::GameWorld(GameState* gs) :
		gs(gs),
		next_room_id(-1) {
	midstep = false;
	lvl = NULL;
}

GameWorld::~GameWorld() {
	for (int i = 0; i < level_states.size(); i++) {
		delete level_states[i];
	}
	level_states.clear();
}

void GameWorld::generate_room(GameRoomState* level) {

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
	GameRoomState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		GameRoomState* lvl = level_states[i];
		serializer.write( Size(lvl->width(), lvl->height()) );
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

	GameRoomState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		Size size;
		serializer.read(size);
		if (level_states[i] == NULL) {
			level_states[i] = new GameRoomState(i, size, false, false);
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
		int myclassn = gs->game_settings().class_type;

		for (int i = 0; i < gs->player_data().all_players().size(); i++) {
			PlayerDataEntry& pde = gs->player_data().all_players()[i];
			bool islocal = &pde == &gs->player_data().local_player_data();
			ClassEntry& c = game_class_data.at(pde.classtype);
			int spriteidx = gs->rng().rand(c.sprites.size());

			if (pde.player_inst.empty()) {
				pde.player_inst = new PlayerInst(c.starting_stats,
						c.sprites[spriteidx], 0, 0, islocal);
			}
			printf("Spawning for player %d: %s\n", i,
					islocal ? "local player" : "network player");
			place_inst(genlevel, pde.player_inst.get());
		}
	} else {
		for (int i = 0; i < nplayers; i++) {
			place_inst(genlevel, (GameInst*)player_instances[i]);
		}
	}
}
GameRoomState* GameWorld::get_level(int roomid, bool spawnplayer,
		void** player_instances, size_t nplayers) {
	GameRoomState* currlvl = gs->get_level(); // Save level context

	if (roomid >= level_states.size()) {
		level_states.resize(roomid + 1, NULL);
	}
	if (!level_states[roomid]) {
		GeneratedLevel genlevel;
		GameRoomState* newlvl = generate_level(roomid, gs->rng(), genlevel,
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

bool GameWorld::pre_step() {
	if (!gs->update_iostate())
		return false;

	GameRoomState* current_level = gs->get_level();

	midstep = true;

	/* Queue actions for local player */
	/* This will result in a network send of the player's actions */
	PlayerData& pd = gs->player_data();
	pd.local_player()->enqueue_io_actions(gs);

	gs->set_level(current_level);
	midstep = false;
	return true;
}

bool GameWorld::step() {
	redofirststep: //I used a goto dont kill mes

	GameRoomState* current_level = gs->get_level();

	midstep = true;

	/* Queue all actions for players */
	/* This will result in a network poll for other players actions
	 * Return false on network failure */
	if (!players_poll_for_actions(gs)) {
		return false;
	}

	for (int i = 0; i < level_states.size(); i++) {
		GameRoomState* level = level_states[i];
		level->step(gs);
	}

	midstep = false;
	if (next_room_id == -2) {
		reset(0);
		next_room_id = 0;
	}
	if (next_room_id != -1) {
		set_current_level(next_room_id);
		gs->view().sharp_center_on(gs->local_player()->pos());
		next_room_id = -1;
	}

	return true;
}

void GameWorld::regen_level(int roomid) {
	GameRoomState* level = get_level(roomid);
	std::vector<PlayerInst*> players = player_data().players_in_level(roomid);

	/* Take all players out of level*/
	for (int i = 0; i < players.size(); i++) {
		// Get and retain player
		players[i]->core_stats().heal_fully();

		// Remove from current level
		gs->remove_instance(players[i]);
	}

	level_states[roomid] = NULL;

	GameRoomState* newlevel = get_level(roomid, true, (void**)&players[0],
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
	GameRoomState* last = gs->get_level();
	GameRoomState* state = get_level(roomid1);
	//set the level context
	gs->set_level(state);

	//Keep object alive during move
	GameInstRef gref(gs->get_instance(id));
	GameInst* inst = gref.get();

	if (!gref.get()) {
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
	std::vector<GameRoomState*> delete_list;
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
		gs->renew_game_timestamp();
	}
	for (int i = 0; i < delete_list.size(); i++) {
		delete delete_list[i];
	}
}

void GameWorld::connect_entrance_to_exit(int roomid1, int roomid2) {
	GameRoomState* l1 = get_level(roomid1);
	GameRoomState* l2 = get_level(roomid2);
	LANARTS_ASSERT(l2->exits.size() == l1->entrances.size());
	for (int i = 0; i < l2->exits.size(); i++) {
		l2->exits[i].exitsqr = l1->entrances[i].entrancesqr;
		l1->entrances[i].exitsqr = l2->exits[i].entrancesqr;
	}
}
