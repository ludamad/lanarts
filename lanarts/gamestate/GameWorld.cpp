/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include <typeinfo>

#include "draw/TileEntry.h"

#include "objects/enemy/EnemyInst.h"

#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"
#include "objects/GameInstRef.h"
#include "stats/ClassEntry.h"

#include <lcommon/math_util.h>

#include "GameMapState.h"
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

void GameWorld::generate_room(GameMapState* level) {

}

level_id GameWorld::get_current_level_id() {
	return gs->get_level()->id();
}

void GameWorld::serialize(SerializeBuffer& serializer) {
	_enemies_seen.serialize(serializer);

	serializer.write_int(level_states.size());
	GameMapState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		GameMapState* lvl = level_states[i];
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

	GameMapState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		Size size;
		serializer.read(size);
		if (level_states[i] == NULL) {
			level_states[i] = new GameMapState(i, size, false, false);
		}
		gs->set_level(level_states[i]);
		level_states[i]->deserialize(gs, serializer);
	}

	gs->set_level(original);
	midstep = false;
}

GameMapState* GameWorld::map_create(const Size& size, bool wandering_enabled) {
	int levelid = level_states.size();
	GameMapState* map = new GameMapState(levelid, Size(size.w * TILE_SIZE, size.h * TILE_SIZE), wandering_enabled);
	level_states.push_back(map);
	return map;
}

void GameWorld::place_player(GameMapState* map, GameInst* p) {
	GameMapState* currlvl = gs->get_level(); // Save level context
	set_current_level(map);
	GameTiles& tiles = map->tiles();
	Pos ppos;
	do {
		int rand_x = gs->rng().rand(tiles.tile_width());
		int rand_y = gs->rng().rand(tiles.tile_height());
		ppos = centered_multiple(Pos(rand_x, rand_y), TILE_SIZE);
	} while (gs->solid_test(NULL, ppos.x, ppos.y, 15));

	p->last_x = ppos.x;
	p->last_y = ppos.y;
	p->update_position(ppos.x, ppos.y);
	gs->add_instance(map->id(), p);
	set_current_level(currlvl);

}

void GameWorld::spawn_players(GameMapState* map, const std::vector<Pos>& positions) {
	bool flocal = (gs->game_settings().conntype == GameSettings::CLIENT);
	GameSettings& settings = gs->game_settings();
	GameNetConnection& netconn = gs->net_connection();
	int myclassn = gs->game_settings().class_type;

	for (int i = 0; i < gs->player_data().all_players().size(); i++) {
		Pos position = positions.at(i);
		PlayerDataEntry& pde = gs->player_data().all_players()[i];
		bool islocal = &pde == &gs->player_data().local_player_data();
		ClassEntry& c = game_class_data.at(pde.classtype);
		int spriteidx = gs->rng().rand(c.sprites.size());

//		if (pde.player_inst.empty()) {
			pde.player_inst = new PlayerInst(c.starting_stats,
					c.sprites[spriteidx], position.x, position.y, islocal);
//		}
		printf("Spawning for player %d: %s\n", i,
				islocal ? "local player" : "network player");
		map->add_instance(gs, pde.player_inst.get());
	}
}

GameMapState* GameWorld::get_level(level_id id) {
	return level_states.at(id);
}

bool GameWorld::pre_step() {
	if (!gs->update_iostate())
		return false;

	GameMapState* current_level = gs->get_level();

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
	GameMapState* current_level = gs->get_level();

	midstep = true;

	/* Queue all actions for players */
	/* This will result in a network poll for other players actions
	 * Return false on network failure */
	if (!players_poll_for_actions(gs)) {
		return false;
	}

	for (int i = 0; i < level_states.size(); i++) {
		GameMapState* level = level_states[i];
		level->step(gs);
	}

	midstep = false;
	if (next_room_id == -2) {
		gs->restart();
		next_room_id = 0;
	}
	if (next_room_id != -1) {
		set_current_level(next_room_id);
		gs->view().sharp_center_on(gs->local_player()->ipos());
		next_room_id = -1;
	}

	return true;
}

void GameWorld::level_move(int id, int x, int y, int roomid1, int roomid2) {
	//save the level context
	GameMapState* last = gs->get_level();
	GameMapState* state = get_level(roomid1);
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
	std::vector<GameMapState*> delete_list;
	if (midstep) {
		next_room_id = -2;
	} else {
		for (int i = keep; i < level_states.size(); i++) {
			delete_list.push_back(level_states[i]);
		}
		player_data().remove_all_players(gs);
		level_states.resize(keep);
		gs->game_hud().override_sidebar_contents(NULL);
		gs->game_chat().clear();
		gs->renew_game_timestamp();
	}
	for (int i = 0; i < delete_list.size(); i++) {
		delete delete_list[i];
	}
}

void GameWorld::connect_entrance_to_exit(int roomid1, int roomid2) {
	GameMapState* l1 = get_level(roomid1);
	GameMapState* l2 = get_level(roomid2);
	LANARTS_ASSERT(l2->exits.size() == l1->entrances.size());
	for (int i = 0; i < l2->exits.size(); i++) {
		l2->exits[i].exitsqr = l1->entrances[i].entrancesqr;
		l1->entrances[i].exitsqr = l2->exits[i].entrancesqr;
	}
}
