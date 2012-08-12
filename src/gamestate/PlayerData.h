/*
 * PlayerData.h:
 *	Holds lists of all players in game, and pointers to their respective PlayerInst's
 */

#ifndef PLAYERCONTROLLER_H_
#define PLAYERCONTROLLER_H_

#include <vector>

#include "ActionQueue.h"

#include "../stats/EnemiesSeen.h"

#include "../objects/GameInst.h"
#include "../objects/GameInstRef.h"

#include "../serialize/SerializeBuffer.h"

class PlayerInst;

struct PlayerDataEntry {
	std::string player_name;
	GameInstRef player_inst;
	int net_id;

	PlayerInst* player();

	PlayerDataEntry(const std::string& player_name, GameInst* player_inst,
			int net_id) :
			player_name(player_name), player_inst(player_inst), net_id(net_id) {
	}
};

//Global data
class PlayerData {
public:
	void update_fieldsofview(GameState* gs);
	void clear();

	void register_player(const std::string& name, PlayerInst* player,
			int net_id);
	PlayerInst* local_player();
	PlayerDataEntry& local_player_data();

	std::vector<PlayerDataEntry>& all_players() {
		return _players;
	}
	bool level_has_player(level_id level);
	std::vector<PlayerInst*> players_in_level(level_id level);

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
