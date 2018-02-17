/*
 * GameWorld.cpp:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#include <typeinfo>

#include <luawrap/luawrap.h>
#include "draw/TileEntry.h"

#include "objects/EnemyInst.h"

#include "objects/PlayerInst.h"
#include "objects/AnimatedInst.h"
#include "objects/GameInstRef.h"
#include "stats/ClassEntry.h"

#include <lcommon/math_util.h>
#include <objects/InstTypeEnum.h>

#include "lanarts_defines.h"

#include "GameMapState.h"
#include "GameState.h"
#include "GameWorld.h"
#include "ScoreBoard.h"

GameWorld::GameWorld(GameState* gs) :
		gs(gs),
		next_room_id(-1) {
	lua_level_states = LuaValue::newtable(gs->luastate());
	midstep = false;
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

void GameWorld::serialize(SerializeBuffer& sb) {
    // (1) Serialize seen enemy data
	_enemies_seen.serialize(sb);

    // (2) Serialize hold-over GameInst's that are destroyed in the game,
    // but still referenced
    sb.write_container(_alive_removed_objects, [&](const GameInstRef& ref) {
        GameInst* inst = ref.get();
        sb.write_int(get_inst_type(inst));
        LANARTS_ASSERT(inst->id <= 0);
        LANARTS_ASSERT((-inst->id) < _alive_removed_objects.size());
        sb.write_int(inst->id);
        inst->serialize(gs, sb);
    });
    for (GameInstRef& ref : _alive_removed_objects) {
        ref->serialize_lua(gs, sb);
    }

    // (3) Serialize level data
    sb.write_int(level_states.size());
	GameMapState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		GameMapState* lvl = level_states[i];
		sb.write( Size(lvl->width(), lvl->height()) );
		gs->set_level(lvl);
		lvl->serialize(gs, sb);
	}

	gs->set_level(original);

    // (4) Serialize team data
    team_data().serialize(gs, sb);
}

void GameWorld::deserialize(SerializeBuffer& sb) {
    // (1) Deserialize seen enemy data
    _enemies_seen.deserialize(sb);

    // (2) Deserialize hold-over GameInst's that are destroyed in the game,
    // but still referenced
    sb.read_container(_alive_removed_objects, [&](GameInstRef& ref) {
        InstType type;
        int id;
        sb.read_int(type);
        sb.read_int(id);
        GameInst* inst = from_inst_type(type);
        inst->id = id;
        inst->deserialize(gs, sb);
        inst->last_x = inst->x;
        inst->last_y = inst->y;
        ref = GameInstRef(inst);
    });
    for (GameInstRef& ref : _alive_removed_objects) {
        ref->deserialize_lua(gs, sb);
        LANARTS_ASSERT(ref->id <= 0);
        LANARTS_ASSERT((-ref->id) < _alive_removed_objects.size());
    }

    // (3) Deserialize level data
    int nlevels;
	sb.read_int(nlevels);
	for (int i = 0; i < level_states.size(); i++) {
		delete level_states.at(i);
                level_states.at(i) = NULL;
	}
	level_states.resize(nlevels, NULL);

	GameMapState* original = gs->get_level();

	for (int i = 0; i < level_states.size(); i++) {
		Size size;
		sb.read(size);
		if (level_states[i] == NULL) {
			level_states[i] = new GameMapState(i, ldungeon_gen::MapPtr(), size, false, false);
		}
		gs->set_level(level_states[i]);
		level_states[i]->deserialize(gs, sb);
	}


    gs->set_level(original);
	midstep = false;

    // (4) Deserialize team data
    team_data().deserialize(gs, sb);

    // (5) Set the current level on each screen based on the local player
    gs->for_screens([&]() {
        GameMapState *level = gs->local_player()->get_map(gs);
        if (level != get_current_level()) {
            set_current_level(level);
        }
    });
}

GameMapState* GameWorld::map_create(const Size& size, ldungeon_gen::MapPtr source_map, bool wandering_enabled) {
	int levelid = level_states.size();
	GameMapState* map = new GameMapState(levelid, source_map, Size(size.w * TILE_SIZE, size.h * TILE_SIZE), wandering_enabled);
	level_states.push_back(map);
        // _should_sync_states = true;
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
	GameSettings& settings = gs->game_settings();

	for (int i = 0; i < gs->player_data().all_players().size(); i++) {
		Pos position = positions.at(i);
		PlayerDataEntry& pde = gs->player_data().all_players()[i];
		ClassEntry& c = game_class_data.get(pde.classtype);
		int spriteidx = gs->rng().rand(c.sprites.size());

//		if (pde.player_inst.empty()) {
        pde.player_inst = new PlayerInst(c.starting_stats,
                c.sprites[spriteidx], position, pde.team, pde.is_local_player);
        if (!pde.input_source.empty()) {
            pde.input_source.push();
            LuaValue input_source = luawrap::call<LuaValue>(gs->luastate(), pde.player_inst.get());
            dynamic_cast<PlayerInst&>(*pde.player_inst.get()).input_source().value = input_source;
        }
//		}
		printf("Spawning for player %d: %s\n", i,
			   pde.is_local_player ? "local player" : "network player");
		map->add_instance(gs, pde.player_inst.get());
	}
    gs->for_screens([&]() {
        GameMapState *level = gs->local_player()->get_map(gs);
        if (level != get_current_level()) {
            set_current_level(level);
        }
    });
}

GameMapState* GameWorld::get_level(level_id id) {
	return level_states.at(id);
}

void GameWorld::set_current_level(GameMapState* level) {
	return gs->screens.set_current_level(level);
}

bool GameWorld::pre_step(bool update_iostate) {
	if (update_iostate && !gs->update_iostate())
		return false;

	GameMapState* current_level = gs->get_level();

	midstep = true;

    gs->screens.for_each_screen( [&]() {
        /* Queue actions for local player */
        /* This will result in a network send of the player's actions */
        if (gs->local_player()) {
            gs->local_player()->enqueue_io_actions(gs);
        }
    });

	set_current_level(current_level);
	midstep = false;
	return true;
}

bool GameWorld::step() {
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
        gs->start_game();
		next_room_id = -1;
                // Don't increment frame number because we're doing a new game:
                return true;
	}
	gs->for_screens([&]() {
        GameMapState* level = gs->focus_object()->get_map(gs);
        if (level != get_current_level()) {
            set_current_level(level);
			// Ensure the view is up to date before view operations:
			LANARTS_ASSERT(level->width() == gs->view().world_width && level->height() == gs->view().world_height);
			Pos diff = (gs->focus_object()->ipos() - gs->screens.screen().last_player_pos);
			gs->view().sharp_move(diff);
			// Ensure that the view is centered. Having the view move after due to being at the edge of a level is jarring:
			for (int i = 0; i< 100;i++) {
				gs->view().center_on(gs->focus_object()->ipos(), 10);
			}
        }
        gs->screens.screen().last_player_pos = gs->focus_object()->ipos();
	});
	// TODO performance
	std::vector<GameInstRef> new_keep_alives;
	for (GameInstRef ref : _alive_removed_objects) {
		if (ref.get()->reference_count > 1) {
			ref->id = -int(new_keep_alives.size());
			new_keep_alives.push_back(ref);
		}
	}
	_alive_removed_objects.swap(new_keep_alives);

	gs->frame()++;
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
        LuaValue lua_object = inst->lua_variables;// stash lua object;

	gs->remove_instance(inst, /* Dont trigger on-destroy effects */ false);
	gref->last_x = x, gref->last_y = y;
	gref->update_position(x, y);

	gs->set_level(get_level(roomid2));
        inst->id = 0;
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
        inst->lua_variables = lua_object;
}

void GameWorld::set_current_level(int roomid) {
	if (midstep)
		set_current_level_lazy(roomid);
	else
		set_current_level(get_level(roomid));
}
void GameWorld::set_current_level_lazy(int roomid) {
	next_room_id = roomid;
}

void GameWorld::lazy_reset() {
    next_room_id = -2;
}
void GameWorld::reset() {
	if (midstep) {
            lazy_reset();
            return;
	}
    _alive_removed_objects.clear(); // TODO investigate " Apparently harmful; dont do this. Better to allow serialization have access to objects from previous runs."
	std::vector<GameMapState*> delete_list = level_states;
        player_data().remove_all_players(gs);
        level_states.clear();
        gs->for_screens([&]() {
            gs->game_hud().override_sidebar_contents(NULL);
            gs->game_chat().clear();
        });
        gs->frame() += 128; // Ensure we don't have any hold-over frames sent.
        // TODO find a better solution than just ensuring unique frame number.
        for (GameMapState* map : delete_list) {
            delete map;
        }
        _team_data = TeamData();
        lvl = NULL;
        // _should_sync_states = true;
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

void GameWorld::push_level_object(level_id id) {
	lua_State* L = gs->luastate();
	if (this->lua_level_states[id].isnil()) {
		lua_newtable(L);
		LuaStackValue(L, -1)["_id"] = id;
		pop_level_object(id);
	}
	this->lua_level_states[id].push();
}

void GameWorld::pop_level_object(level_id id) {
	this->lua_level_states[id].pop();
}

void GameWorld::spawn_players(GeneratedRoom& genlevel, void** player_instances,
		size_t nplayers) {
}

GameMapState *GameWorld::get_current_level() {
	return gs->screens.get_current_level();
}

void GameWorld::register_removed_object(GameInst* inst) {
	inst->id = -int(_alive_removed_objects.size());
	_alive_removed_objects.push_back({inst});
}

GameInstRef &GameWorld::get_removed_object(int id) {
	return _alive_removed_objects.at(id);
}
