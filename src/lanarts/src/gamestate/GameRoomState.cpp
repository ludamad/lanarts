/*
 * GameLevelState.cpp:
 *  Contains game level state information
 */

#include "objects/player/PlayerInst.h"

#include <lcommon/SerializeBuffer.h>

#include "GameRoomState.h"

#include "GameState.h"

GameRoomState::GameRoomState(int levelid, const Size& size,
		bool wandering_flag, bool is_simulation) :
		_levelid(levelid),
		_steps_left(0),
		_size(size),
		_tiles(Size(size.w / TILE_SIZE, size.h / TILE_SIZE)),
		_inst_set(size.w, size.h),
		_monster_controller(wandering_flag),
		_is_simulation(is_simulation),
		_wander_map() {
}

GameRoomState::~GameRoomState() {
}

void GameRoomState::copy_to(GameRoomState & room) const {
	room.entrances = this->entrances; //Copy exits&entrances just in case
	room.exits = this->exits; //However we will typically copy_to just to synch
//	this->inst_set.copy_to(level.inst_set);
//	level.is_simulation = this->is_simulation;
//	tiles.copy_to(level.tiles);
//	this->mc.partial_copy_to(level.mc);
//	level.mc.finish_copy(&level);
//	level.is_simulation = this->is_simulation;
//	level.steps_left = this->steps_left;
}

int GameRoomState::room_within(const Pos& p) {
	for (int i = 0; i < rooms.size(); i++) {
		int px = p.x / TILE_SIZE, py = p.y / TILE_SIZE;
		const Region & r = rooms[i].region;
		if (r.x <= px && r.x + r.w >= px) {
			if (r.y <= py && r.y + r.h >= py) {
				return i;
			}
		}

	}
	return -1;
}

GameRoomState* GameRoomState::clone() const {
	GameRoomState* state = new GameRoomState(_levelid, _size, _is_simulation);
	copy_to(*state);
	return state;
}

static void update_player_fields_of_view(GameState* gs) {
	std::vector<PlayerInst*> players = gs->players_in_level();
	for (int i = 0; i < players.size(); i++) {
		players[i]->update_field_of_view(gs);
	}
}

void GameRoomState::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_container(exits);
	serializer.write_container(entrances);
	serializer.write_container(rooms);
	serializer.write(_levelid);
	serializer.write(_steps_left);
	serializer.write(_size);
	serializer.write(_is_simulation);

	tiles().serialize(serializer);
	game_inst_set().serialize(gs, serializer);
	monster_controller().serialize(serializer);
}

void GameRoomState::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_container(exits);
	serializer.read_container(entrances);
	serializer.read_container(rooms);
	serializer.read(_levelid);
	serializer.read(_steps_left);
	serializer.read(_size);
	serializer.read(_is_simulation);

	tiles().deserialize(serializer);
	collision_avoidance().clear();
	game_inst_set().deserialize(gs, serializer);
	monster_controller().deserialize(serializer);

}

void GameRoomState::step(GameState* gs) {
	const int STEPS_TO_SIMULATE = 1000;

	bool has_player_in_level = gs->player_data().level_has_player(id());

	if (has_player_in_level) {
		_steps_left = STEPS_TO_SIMULATE;
	}
	if (_steps_left <= 0) {
		return;
	}
	perf_timer_begin(FUNCNAME);

	GameRoomState* previous_level = gs->get_level();
	gs->set_level(this);

	update_player_fields_of_view(gs);
	monster_controller().pre_step(gs);
	game_inst_set().step(gs);
	tiles().step(gs);
	_steps_left--;

	gs->set_level(previous_level);

	perf_timer_end(FUNCNAME);
}
