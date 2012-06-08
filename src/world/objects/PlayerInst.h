/* PlayerInst.h:
 *  Represents a player of the game, in a networked game there can be many, only one of
 *  which is the 'local' player.
 *  See player_impl/ for member implementations.
 */

#ifndef PLAYERINST_H_
#define PLAYERINST_H_
#include <deque>

#include "../../fov/fov.h"

#include "../../data/sprite_data.h"

#include "../../gamestats/Inventory.h"
#include "../../gamestats/effects.h"
#include "../../gamestats/Equipment.h"
#include "../../gamestats/stats.h"

#include "../../pathfind/pathfind.h"

#include "../GameAction.h"
#include "../GameLevelState.h"

#include "GameInst.h"

const int REST_COOLDOWN = 300;

struct PlayerCooldowns {
	int canrestcooldown;
	int canpickupcooldown;
	PlayerCooldowns() : canrestcooldown(0), canpickupcooldown(0) {
	}
	void step(){
		if (--canrestcooldown < 0)
			canrestcooldown = 0;
		if (--canpickupcooldown < 0)
			canpickupcooldown = 0;
	}
	bool can_pickup(){
		return canpickupcooldown <= 0;
	}
	bool can_rest(){
		return canrestcooldown <= 0;
	}
};

class PlayerInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1, DEPTH = 75
	};
	PlayerInst(const Stats& start_stats, int x, int y, bool local = true) :
			GameInst(x, y, RADIUS, true, DEPTH), local(local), isresting(0), base_stats(
					start_stats), money(0), spellselect(0) {
	}

	virtual ~PlayerInst();
	virtual void init(GameState *gs);
	virtual void deinit(GameState *gs);
	virtual void step(GameState *gs);
	virtual void draw(GameState *gs);
	virtual void copy_to(GameInst* inst) const;
	virtual PlayerInst* clone() const;

	void gain_xp(GameState* gs, int xp);
	void equip(item_id item, int amnt = 1);

	void queue_io_actions(GameState* gs);
	void queue_io_spell_and_attack_actions(GameState* gs, float dx, float dy);
	void queue_io_equipment_actions(GameState* gs);
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

	EffectStats & status_effects() {
		return effects;
	}

	int& spell_selected() {
		return spellselect;
	}
	int& rest_cooldown() {
		return cooldowns.canrestcooldown;
	}
	void reset_rest_cooldown(int cooldown = REST_COOLDOWN) {
		rest_cooldown() = std::max(cooldown, rest_cooldown());
	}
	int gold() {
		return money;
	}
	int& weapon_type() {
		return equipment.weapon;
	}

	bool is_local_player() {
		return local;
	}
	Inventory& get_inventory() {
		return equipment.inventory;
	}
	Equipment& get_equipment() {
		return equipment;
	}

	bool& performed_actions_for_step() {
		return didstep;
	}

private:
	void use_move(GameState* gs, const GameAction& action);
	void use_weapon(GameState* gs, const GameAction& action);
	void use_dngn_exit(GameState* gs, const GameAction& action);
	void use_dngn_entrance(GameState* gs, const GameAction& action);
	void use_spell(GameState* gs, const GameAction& action);
	void use_rest(GameState* gs, const GameAction& action);
	void use_item(GameState* gs, const GameAction& action);
	void pickup_item(GameState* gs, const GameAction& action);
	void drop_item(GameState* gs, const GameAction& action);
	void reposition_item(GameState* gs, const GameAction& action);

	std::deque<GameAction> queued_actions;
	bool didstep;
	bool local, isresting;
	PlayerCooldowns cooldowns;
	Equipment equipment;
	Stats base_stats;
	EffectStats effects;

	int money;
	int spellselect;
};

#endif /* PLAYERINST_H_ */
