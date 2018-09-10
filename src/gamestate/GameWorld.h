/* GameWorld:
 *  Contains player object information
 *  Contains enemy's seen information
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated.
 */

#ifndef GAMEWORLD_H_
#define GAMEWORLD_H_

#include <vector>
#include <cstdlib>

#include <luawrap/LuaValue.h>

#include <ldungeon_gen/Map.h>
#include <lcommon/lambda_utils.h>

#include "objects/TeamRelations.h"

#include "stats/EnemiesSeen.h"

#include "lanarts_defines.h"

#include "PlayerData.h"
#include "Team.h"
#include "GameState.h"

class GameState;
class GameMapState;
class GeneratedRoom;
class PlayerInst;

class GameWorld {
public:
	GameWorld(GameState* gs);
	~GameWorld();
	void generate_room(GameMapState* level);
	GameMapState* get_level(level_id id);
	bool pre_step(bool update_iostate = true);
	bool step();
	void level_move(int id, int x, int y, int roomid1, int roomid2);
	void set_current_level(int roomid);
	void set_current_level_lazy(int roomid);

	GameMapState* map_create(const Size& size, ldungeon_gen::MapPtr source_map, bool wandering_enabled = true);

	void reset();
	void lazy_reset();
	GameMapState* get_current_level();
	void set_current_level(GameMapState* level);
	PlayerData& player_data() {
		return _player_data;
	}
	EnemiesSeen& enemies_seen() {
		return _enemies_seen;
	}

	void push_level_object(level_id id);
	void pop_level_object(level_id id);

	template <typename Func>
	bool for_each(const Func& func) {
		return ::for_each(level_states, func);
	}

	int number_of_levels() {
		return level_states.size();
	}
	int get_current_level_id();
	void connect_entrance_to_exit(int roomid1, int roomid2);
	void serialize(SerializeBuffer& sb);
	void deserialize(SerializeBuffer& serializer);
        
        bool& should_sync_states() {
            return _should_sync_states;
        }

	void spawn_players(GameMapState* map, const std::vector<Pos>& positions);
        TeamData& team_data() {
            return _team_data;
        }
    void register_removed_object(GameInst* inst);
	GameInstRef& get_removed_object(int id);
	std::vector<GameMapState*>& maps() {
		return level_states;
	}
private:
	void place_player(GameMapState* map, GameInst* p);
	bool midstep;
	int next_room_id;

	EnemiesSeen _enemies_seen;
	PlayerData _player_data;

	GameMapState* lvl;
	GameState* gs;
	LuaValue lua_level_states;
	bool _should_sync_states = false;
	// All removed objects that are still held on to by game state.
	std::vector<GameInstRef> _alive_removed_objects;
	std::vector<GameMapState*> level_states;
	TeamData _team_data;
};

#endif /* GAMEWORLD_H_ */
