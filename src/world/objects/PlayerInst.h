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

#include "../../pathfind/pathfind.h"

#include "../../util/ActionQueue.h"

#include "../GameAction.h"
#include "../GameLevelState.h"

#include "CombatGameInst.h"
#include "GameInst.h"

const int REST_COOLDOWN = 200;

class PlayerInst: public CombatGameInst {
public:
	enum {
		RADIUS = 10, DEPTH = 75, LINEOFSIGHT = 7
	};
	PlayerInst(const CombatStats& stats, sprite_id sprite, int x, int y,
			bool local = true) :
			CombatGameInst(stats, sprite, 0, x, y, RADIUS, true, DEPTH), fieldofview(
					LINEOFSIGHT), local(local), moving(0), money(0), lives(0), kills(
					0), deaths(0), previous_spellselect(0), spellselect(-1) {
	}

	virtual ~PlayerInst();
	virtual void init(GameState *gs);
	virtual void die(GameState* gs);
	virtual void deinit(GameState *gs);
	virtual void step(GameState *gs);
	virtual void draw(GameState *gs);
	virtual void copy_to(GameInst* inst) const;
	virtual PlayerInst* clone() const;

	void gain_xp(GameState* gs, int xp);

	void queue_io_actions(GameState* gs);

	void perform_queued_actions(GameState* gs);
	void perform_action(GameState* gs, const GameAction& action);

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
	int gold() {
		return money;
	}
	int number_of_deaths() {
		return deaths;
	}
	int number_of_kills() {
		return kills;
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

	virtual void signal_killed_enemy() {
		kills++;
	}

	bool& performed_actions_for_step() {
		return didstep;
	}

	fov& field_of_view() {
		return fieldofview;
	}

private:

	/* Deciding which actions to take in a step */
	void queue_io_movement_actions(GameState* gs, int& dx, int& dy);
	bool queue_io_spell_actions(GameState* gs);
	bool queue_io_spell_and_attack_actions(GameState* gs, float dx, float dy);
	void queue_io_equipment_actions(GameState* gs, bool do_stop_action);
	void queue_network_actions(GameState* gs);

	/* Performing Actions */
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

	/* Members */
	ActionQueue queued_actions;
	fov fieldofview;
	bool didstep, local, moving;

	int money, lives, deaths, kills;
	int previous_spellselect, spellselect;
};

bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position);

#endif /* PLAYERINST_H_ */
