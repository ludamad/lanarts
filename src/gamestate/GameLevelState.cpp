/*
 * GameLevelState.cpp:
 *  Contains game level state information
 */

#include "GameLevelState.h"

#include "GameState.h"

#include "../serialize/SerializeBuffer.h"

#include "../objects/player/PlayerInst.h"

GameLevelState::GameLevelState(int levelid, int w, int h, bool wandering_flag,
		bool is_simulation) :
		_levelid(levelid), _steps_left(0), _width(w), _height(h), _tiles(
				w / TILE_SIZE, h / TILE_SIZE), _inst_set(w, h), _monster_controller(
				wandering_flag), _is_simulation(is_simulation) {
}

GameLevelState::~GameLevelState() {
}

void GameLevelState::copy_to(GameLevelState & level) const {
	level.entrances = this->entrances; //Copy exits&entrances just in case
	level.exits = this->exits; //However we will typically copy_to just to synch
//	this->inst_set.copy_to(level.inst_set);
//	level.is_simulation = this->is_simulation;
//	tiles.copy_to(level.tiles);
//	this->mc.partial_copy_to(level.mc);
//	level.mc.finish_copy(&level);
//	level.is_simulation = this->is_simulation;
//	level.steps_left = this->steps_left;
}

int GameLevelState::room_within(const Pos& p) {
	for (int i = 0; i < rooms.size(); i++) {
		int px = p.x / TILE_SIZE, py = p.y / TILE_SIZE;
		const Region & r = rooms[i].room_region;
		if (r.x <= px && r.x + r.w >= px) {
			if (r.y <= py && r.y + r.h >= py) {
				return i;
			}
		}

	}
	return -1;
}

GameLevelState* GameLevelState::clone() const {
	GameLevelState* state = new GameLevelState(_levelid, _width, _height,
			_is_simulation);
	copy_to(*state);
	return state;
}

static void update_player_fields_of_view(GameState* gs) {
	std::vector<PlayerInst*> players = gs->players_in_level();
	for (int i = 0; i < players.size(); i++) {
		players[i]->update_field_of_view(gs);
	}
}

void GameLevelState::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_container(exits);
	serializer.write_container(entrances);
	serializer.write_container(rooms);
	serializer.write(_levelid);
	serializer.write(_steps_left);
	serializer.write(_width);
	serializer.write(_height);
	serializer.write(_is_simulation);

	tiles().serialize(serializer);
	game_inst_set().serialize(gs, serializer);
	monster_controller().serialize(serializer);
}

void GameLevelState::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_container(exits);
	serializer.read_container(entrances);
	serializer.read_container(rooms);
	serializer.read(_levelid);
	serializer.read(_steps_left);
	serializer.read(_width);
	serializer.read(_height);
	serializer.read(_is_simulation);

	tiles().deserialize(serializer);
	collision_avoidance().clear();
	game_inst_set().deserialize(gs, serializer);
	monster_controller().deserialize(serializer);

}

void GameLevelState::step(GameState* gs) {
	const int STEPS_TO_SIMULATE = 1000;

	bool has_player_in_level = gs->player_data().level_has_player(id());

	if (has_player_in_level) {
		_steps_left = STEPS_TO_SIMULATE;
	}
	if (_steps_left <= 0) {
		return;
	}

	GameLevelState* previous_level = gs->get_level();
	gs->set_level(this);

	update_player_fields_of_view(gs);
	monster_controller().pre_step(gs);
	game_inst_set().step(gs);
	tiles().step(gs);
	_steps_left--;

	gs->set_level(previous_level);
}
