/* PlayerInst.h:
 *  Represents a player of the game, in a networked game there can be many, only one of
 *  which is the 'local' player.
 *  See player_impl/ for member implementations.
 */

#ifndef PLAYERINST_H_
#define PLAYERINST_H_

#include <deque>

#include "../../fov/fov.h"

#include "../../display/sprite_data.h"

#include "../../pathfind/pathfind.h"

#include "../../gamestate/ActionQueue.h"

#include "../../gamestate/GameAction.h"
#include "../../gamestate/GameLevelState.h"

#include "../CombatGameInst.h"
#include "../GameInst.h"

const int REST_COOLDOWN = 200;

class PlayerInst: public CombatGameInst {
public:
	enum {
		RADIUS = 10, DEPTH = 75, LINEOFSIGHT = 7
	};
	PlayerInst(const CombatStats& stats, int x, int y, bool local = true);

	virtual ~PlayerInst();
	virtual void init(GameState* gs);
	virtual void die(GameState* gs);
	virtual void deinit(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual PlayerInst* clone() const;
	void gain_xp(GameState* gs, int xp);
	void queue_io_actions(GameState* gs);
	void perform_queued_actions(GameState* gs);
	void perform_action(GameState* gs, const GameAction& action);

	//Field of view
	virtual void update_field_of_view(GameState* gs);
	virtual bool within_field_of_view(const Pos& pos);

	int spell_selected() {
		return spellselect;
	}

	int& rest_cooldown() {
		return stats().cooldowns.rest_cooldown;
	}

	void reset_rest_cooldown(int cooldown = REST_COOLDOWN) {
		stats().cooldowns.reset_rest_cooldown(cooldown);
	}

	money_t& gold() {
		return equipment().money;
	}

	int number_of_deaths() {
		return deaths;
	}

	Weapon& weapon() {
		return equipment().weapon;
	}

	Projectile& projectile() {
		return equipment().projectile;
	}

	bool is_local_player() {
		return local;
	}

	bool& performed_actions_for_step() {
		return didstep;
	}

	fov& field_of_view() {
		return fieldofview;
	}

private:
	void queue_io_movement_actions(GameState* gs, int& dx, int& dy);
	bool queue_io_spell_actions(GameState* gs);
	bool queue_io_spell_and_attack_actions(GameState* gs, float dx, float dy);
	void queue_io_equipment_actions(GameState* gs, bool do_stop_action);
	void queue_network_actions(GameState* gs);
	void queue_not_enough_mana_actions(GameState* gs);

	//Game action events
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
	void purchase_from_store(GameState* gs, const GameAction& action);

	ActionQueue queued_actions;
	fov fieldofview;
	bool didstep, local, moving;

	int autouse_mana_potion_try_count;
	int lives, deaths;
	int previous_spellselect, spellselect;
};

bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position);

#endif /* PLAYERINST_H_ */
