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

#include "../GameAction.h"

const int REST_COOLDOWN = 150;

class PlayerInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1
	};
	PlayerInst(const Stats& start_stats, int x, int y, bool local = true) :
			GameInst(x, y, RADIUS), local(local), isresting(0), weapon(0),
			base_stats(start_stats), canrestcooldown(0),
			money(0), spellselect(0) {
	}

	virtual ~PlayerInst();
	virtual void init(GameState *gs);
	virtual void deinit(GameState *gs);
	virtual void step(GameState *gs);
	virtual void draw(GameState *gs);

	void perform_io_action(GameState* gs);
	void perform_action(GameState* gs, const GameAction& action);

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

	int spell_selected(){
		return spellselect;
	}
	int& rest_cooldown(){
		return canrestcooldown;
	}
	int gold() {
		return money;
	}
	int& weapon_type(){
		return weapon;
	}
	//Is it player of focus ?
	bool is_local_focus(){
		return local;
	}
	Inventory& get_inventory(){
		return inventory;
	}

private:
	void use_move_and_melee(GameState *gs, const GameAction& action);
	void use_dngn_exit(GameState* gs, const GameAction& action);
	void use_dngn_entrance(GameState *gs, const GameAction& action);
	void use_spell(GameState *gs, const GameAction& action);
	void use_rest(GameState *gs, const GameAction& action);
	void use_item(GameState *gs, const GameAction& action);
	void pickup_item(GameState* gs, const GameAction& action);
	void drop_item(GameState* gs, const GameAction& action);

	Inventory inventory;
	bool local, isresting;
	int weapon;
	Stats base_stats;
	Effects effects;
	int canrestcooldown;
	int money;
	int spellselect;
};

#endif /* PLAYERINST_H_ */
