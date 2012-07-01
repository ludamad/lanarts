/*
 * PlayerController.cpp:
 *	Handles network communication of player coordinates, creates input actions for player objects.
 */

#include "PlayerController.h"
#include "../GameTiles.h"
#include "../GameState.h"
#include "../../util/math_util.h"
#include "../../fov/fov.h"

PlayerController::PlayerController() {
}

PlayerController::~PlayerController() {
	for (int i = 0; i < fovs.size(); i++)
		delete fovs[i];
}

void PlayerController::update_fieldsofview(GameState* gs){
	for (int i = 0; i < pids.size(); i++) {
		GameInst* player = gs->get_instance(pids[i]);
		int sx = player->last_x * VISION_SUBSQRS / TILE_SIZE;
		int sy = player->last_y * VISION_SUBSQRS / TILE_SIZE;
		fovs[i]->calculate(gs, sx, sy);
	}
}
void PlayerController::pre_step(GameState* gs) {
	update_fieldsofview(gs);
}

fov* PlayerController::playerfov(obj_id pid){
	int i;
	for (i = 0; i < pids.size(); i++) {
		if (pids[i] == pid)
			break;
	}
	return fovs[i];
}
fov* PlayerController::local_playerfov() {
	return playerfov(local_playerid());
}

void PlayerController::copy_to(PlayerController& pc) const {
	pc.local_player = this->local_player;
	pc.pids.clear();

	for (int i = 0; i < pc.fovs.size(); i++)
		delete pc.fovs[i];
	pc.fovs.clear();

	for (int i = 0; i < this->pids.size(); i++){
		pc.pids.push_back(this->pids[i]);
		pc.fovs.push_back(this->fovs[i]->clone());
	}
}

void PlayerController::register_player(obj_id player, bool islocal) {
	if (islocal)
		local_player = player;
	pids.push_back(player);
//	if (islocal)
		fovs.push_back(new fov(7, VISION_SUBSQRS));
//	else
//		fovs.push_back(new fov(4, VISION_SUBSQRS));

}

void PlayerController::deregister_player(obj_id player) {
	int i;
	for (i = 0; i < pids.size() && pids[i] != player; i++) {
		//find 'i' such that pids[i] == player
	}
	if (i == pids.size()){
        printf("Didn't find player in list\n");
		return;
    }
	pids.erase(pids.begin() + i);
	delete fovs[i];
	fovs.erase(fovs.begin() + i);
    printf("Deregistering '%d'\n", player);
    
    fflush(stdout);
}

bool PlayerController::seen_by_player(GameState* gs, int pindex,
		GameInst* obj) {
	const int sub_sqrs = VISION_SUBSQRS;
	const int subsize = TILE_SIZE / sub_sqrs;

	int w = gs->width() / subsize, h = gs->height() / subsize;
	int x = obj->last_x, y = obj->last_y;
	int rad = obj->radius;
	int mingrid_x = (x - rad) / subsize, mingrid_y = (y - rad) / subsize;
	int maxgrid_x = (x + rad) / subsize, maxgrid_y = (y + rad) / subsize;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);
	fov& pfov = *fovs[pindex];
	//printf("minx=%d,miny=%d,maxx=%d,maxy=%d\n",minx,miny,maxx,maxy);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			if (pfov.within_fov(xx, yy))
				return true;
		}
	}
	return false;
}

void PlayerController::clear(){
	pids.clear();
	for (int i = 0; i < fovs.size(); i++)
		delete fovs[i];
	fovs.clear();
}

