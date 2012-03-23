#ifndef PLAYERINST_H_
#define PLAYERINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../gamestats/Inventory.h"
#include "../../fov/fov.h"
#include "../../pathfind/pathfind.h"
#include "../GameLevelState.h"

class PlayerInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1
	};
	PlayerInst(int x, int y) :
			GameInst(x, y, RADIUS), base_stats(4, 100, 100, true /*canmelee*/,
					true /*canranged*/, 10, 10, 20/*cooldoown*/, 20 /*reach*/,
					400 /*range*/, 5), money(0) {
		portal = NULL;
	}
	virtual ~PlayerInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	Stats& stats() {
		return base_stats;
	}
	int gold() {
		return money;
	}
	Inventory inventory;
	//Hack for now of remembering which entrance we hit last
	GameLevelPortal* portal_used() {
		return portal;
	}
private:
	void move(GameState* gs, int dx, int dy);
	Stats base_stats;
	int money;
	//Hack for now of remembering which entrance we hit last
	GameLevelPortal* portal;
};

#endif /* PLAYERINST_H_ */
