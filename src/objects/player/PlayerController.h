/*
 * PlayerController.h:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#ifndef PLAYERCONTROLLER_H_
#define PLAYERCONTROLLER_H_

#include <vector>

#include "../../stats/EnemiesSeen.h"

#include "../GameInst.h"
#include "../GameInstRef.h"

class PlayerInst;

//One instance for all levels
class PlayerController {
public:
	void update_fieldsofview(GameState* gs);
	void clear();

	void register_player(PlayerInst* player);
	PlayerInst* local_player();

	const std::vector<GameInstRef>& all_players() {
		return _players;
	}
	bool level_has_player(level_id level);
	std::vector<PlayerInst*> players(level_id level);

	void players_gain_xp(GameState* gs, int xp);
	void copy_to(PlayerController& pc) const;
private:
	GameInstRef _local_player;
	std::vector<GameInstRef> _players;
};

#endif /* PLAYERCONTROLLER_H_ */
