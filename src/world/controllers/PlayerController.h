/*
 * PlayerController.h:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#ifndef PLAYERCONTROLLER_H_
#define PLAYERCONTROLLER_H_
#include <vector>
#include "../objects/GameInst.h"

class fov;

class PlayerController {
public:
	PlayerController();
	~PlayerController();
	void update_fieldsofview(GameState* gs);
	void pre_step(GameState* gs);
	void clear();

	void register_player(obj_id player, bool islocal);
	void deregister_player(obj_id player);
	bool seen_by_player(GameState* gs, int pindex, GameInst* obj);

	const std::vector<obj_id>& player_ids() {
		return pids;
	}
	const std::vector<fov*>& player_fovs() {
		return fovs;
	}
	obj_id local_playerid() {
		return local_player;
	}
	bool has_player() {
		return !pids.empty();
	}
	fov* local_playerfov();
	fov* playerfov(obj_id pid);

	void copy_to(PlayerController& pc) const;
private:
	obj_id local_player;
	std::vector<fov*> fovs;
	std::vector<obj_id> pids;
};

#endif /* PLAYERCONTROLLER_H_ */
