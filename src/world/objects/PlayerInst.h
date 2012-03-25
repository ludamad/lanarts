#ifndef PLAYERINST_H_
#define PLAYERINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../gamestats/Inventory.h"
#include "../../gamestats/Effects.h"
#include "../../fov/fov.h"
#include "../../pathfind/pathfind.h"
#include "../GameLevelState.h"

#include "../../data/sprite_data.h"

class PlayerInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1
	};
	PlayerInst(int x, int y) :
			GameInst(x, y, RADIUS), base_stats(4, 100, 100,
					Attack(true, 10, 25, 40),
					Attack(true, 8, 400, 40, SPR_FIREBOLT, 7)), canrestcooldown(
					0), money(0) {
		portal = NULL;
	}

	virtual ~PlayerInst();
	virtual void init(GameState *gs);
	virtual void step(GameState *gs);
	virtual void draw(GameState *gs);
	Stats & stats() {
		return base_stats;
	}

	Stats effective_stats() {
		Stats tmp = base_stats;
		effects.process(base_stats, tmp);
		return tmp;
	}

	Effects & status_effects() {
		return effects;
	}

	int gold() {
		return money;
	}

	Inventory inventory;
	GameLevelPortal *portal_used() {
		return portal;
	}

private:
	void move(GameState *gs, int dx, int dy);
	Stats base_stats;
	Effects effects;
	int canrestcooldown;
	int money;
	GameLevelPortal* portal;
};

#endif /* PLAYERINST_H_ */
