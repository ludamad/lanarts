/*
 * PlayerData.h:
 *	Holds lists of all players in game, and pointers to their respective PlayerInst's
 */

#ifndef PLAYERCONTROLLER_H_
#define PLAYERCONTROLLER_H_

#include <vector>

#include "../objects/GameInst.h"

#include "../objects/GameInstRef.h"

#include "../serialize/SerializeBuffer.h"
#include "../stats/EnemiesSeen.h"

#include "ActionQueue.h"

class PlayerInst;

struct PlayerDataEntry {
	std::string player_name;
	MultiframeActionQueue action_queue;
	GameInstRef player_inst;
	class_id classtype;
	int net_id;

	PlayerInst* player();

	PlayerDataEntry(const std::string& player_name, GameInst* player_inst,
			class_id classtype, int net_id) :
			player_name(player_name), player_inst(player_inst), classtype(
					classtype), net_id(net_id) {
	}
};

//Global data
class PlayerData {
public:
	PlayerData() :
			_local_player_idx(0) {
	}
	void update_fieldsofview(GameState* gs);
	void clear();
	void register_player(const std::string& name, PlayerInst* player,
			class_id classtype, int net_id = 0);
	PlayerInst* local_player();
	PlayerDataEntry& local_player_data();

	std::vector<PlayerDataEntry>& all_players() {
		return _players;
	}
	bool level_has_player(level_id level);
	std::vector<PlayerInst*> players_in_level(level_id level);

	PlayerDataEntry& get_entry_by_netid(int netid);

	void set_local_player_idx(int idx);
	int get_local_player_idx() {
		return _local_player_idx;
	}
	void remove_all_players(GameState* gs);
	void copy_to(PlayerData& pc) const;

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

private:
	int _local_player_idx;
	std::vector<PlayerDataEntry> _players;
};

int player_get_playernumber(GameState* gs, PlayerInst* p);

void players_gain_xp(GameState* gs, int xp);

void players_poll_for_actions(GameState* gs);


#endif /* PLAYERDATA_H_ */
