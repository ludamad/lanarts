/*
 * PlayerController.h:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#ifndef PLAYERCONTROLLER_H_
#define PLAYERCONTROLLER_H_

#include <vector>

#include "../../gamestats/EnemiesSeen.h"

#include "../objects/GameInst.h"
#include "../objects/GameInstRef.h"

class PlayerController {
public:
	PlayerController();
	~PlayerController();
	void update_fieldsofview(GameState* gs);
	void pre_step(GameState* gs);
	void clear();

	void register_player(obj_id player, bool islocal);
	void deregister_player(obj_id player);

	const std::vector<obj_id>& player_ids() {
		return pids;
	}
	obj_id local_playerid() {
		return local_player;
	}
	bool has_player() {
		return !pids.empty();
	}
	void players_gain_xp(GameState* gs, int xp);
	void copy_to(PlayerController& pc) const;
private:
	obj_id local_player;
	std::vector<obj_id> pids;
};

//One instance for all levels
class _PlayerController {
public:
	_PlayerController();
	~_PlayerController();
	void update_fieldsofview(GameState* gs);
	void pre_step(GameState* gs);
	void clear();

	void register_player(obj_id player, bool islocal);
	void deregister_player(obj_id player);
//
//	const std::vector<obj_id>& player_ids() {
//		return pids;
//	}
//	obj_id local_playerid() {
//		return local_player;
//	}
	bool has_player() {
		return !pids.empty();
	}
	void copy_to(PlayerController& pc) const;
private:
	GameInstRef local_player;
	std::vector<GameInstRef> pids;
};

#endif /* PLAYERCONTROLLER_H_ */
