/* PlayerInst.h:
 *  Represents a player of the game, in a networked game there can be many, only one of
 *  which is the 'local' player
 */

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
#include <deque>

const int REST_COOLDOWN = 150;

class PlayerInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1, DEPTH = 75	};
	PlayerInst(const Stats& start_stats, int x, int y, bool local = true) :
			GameInst(x, y, RADIUS, true, DEPTH), local(local), isresting(0), weapon(0),
			base_stats(start_stats), canrestcooldown(0),
			money(0), spellselect(0) {
	}

	virtual ~PlayerInst();
	virtual void init(GameState *gs);
	virtual void deinit(GameState *gs);
	virtual void step(GameState *gs);
	virtual void draw(GameState *gs);
	virtual void copy_to(GameInst* inst) const;
	virtual PlayerInst* clone() const;

	void queue_io_actions(GameState* gs);
	void queue_network_actions(GameState* gs);
	void perform_queued_actions(GameState* gs);
	void perform_action(GameState* gs, const GameAction& action);

	Stats & stats() {
		return base_stats;
	}

	Stats effective_stats(lua_State* L) {
		Stats tmp = base_stats;
		effects.process(L, base_stats, tmp);
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

	//Is this the local player?
	bool is_local_focus(){
		return local;
	}
	Inventory& get_inventory(){
		return inventory;
	}

	bool& performed_actions_for_step(){
		return didstep;
	}

private:
	void use_move(GameState *gs, const GameAction& action);
	void use_weapon(GameState *gs, const GameAction& action);
	void use_dngn_exit(GameState* gs, const GameAction& action);
	void use_dngn_entrance(GameState *gs, const GameAction& action);
	void use_spell(GameState *gs, const GameAction& action);
	void use_rest(GameState *gs, const GameAction& action);
	void use_item(GameState *gs, const GameAction& action);
	void pickup_item(GameState* gs, const GameAction& action);
	void drop_item(GameState* gs, const GameAction& action);

	std::deque<GameAction> queued_actions;
	bool didstep;
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
