/*
 * PlayerData.h:
 *	Holds lists of all players in game, and pointers to their respective PlayerInst's
 */

#ifndef PLAYERCONTROLLER_H_
#define PLAYERCONTROLLER_H_

#include <vector>
#include <lcommon/SerializeBuffer.h>

#include "objects/GameInst.h"

#include "objects/GameInstRef.h"

#include "stats/EnemiesSeen.h"

#include "ActionQueue.h"

class PlayerInst;

struct PlayerDataEntry {
	std::string player_name;
	MultiframeActionQueue action_queue;
	GameInstRef player_inst;
	std::string classtype;
	bool is_local_player;
	int net_id;
    int index; // Index in list, for convenience

	PlayerInst* player() const;

	PlayerDataEntry(const std::string& player_name, GameInst* player_inst,
					const std::string& classtype, bool is_local_player, int net_id, int index) :
			player_name(player_name), player_inst(player_inst), classtype(
					classtype), is_local_player(is_local_player), net_id(net_id), index(index) {
	}
};

//Global data
class PlayerData {
public:
	PlayerData() {
	}
	void clear();
	void register_player(const std::string& name, PlayerInst* player,
			const std::string& classtype, bool is_local_player, int net_id = 0);

	std::vector<PlayerDataEntry>& all_players() {
		return _players;
	}
	bool level_has_player(level_id level);
	std::vector<PlayerInst*> players_in_level(level_id level);

	PlayerDataEntry& get_entry_by_netid(int netid);

	PlayerDataEntry& get(int player_idx) {
		return _players.at(player_idx);
	}
	void remove_all_players(GameState* gs);
	void copy_to(PlayerData& pc) const;

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	int& n_enemy_killed(int type) {
		if (type >= _kill_amounts.size()) {
			_kill_amounts.resize(type + 1, 0);
		}
		return _kill_amounts.at(type);
	}
	money_t& money() {
		return _money;
	}
	void reset() {
		_money = 0;
		_kill_amounts.resize(0);
	}
private:
    // Gold pool for all players:
    money_t _money = 0;
	int _local_player_idx;
	std::vector<PlayerDataEntry> _players;
    // Tracks player kills:
	std::vector<int> _kill_amounts;
};

int player_get_playernumber(GameState* gs, PlayerInst* p);

void players_gain_xp(GameState* gs, int xp);

bool players_poll_for_actions(GameState* gs);


#endif /* PLAYERDATA_H_ */
