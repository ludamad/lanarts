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

const int REST_COOLDOWN = 300;

class PlayerInst: public CombatGameInst {
public:
	enum {
		RADIUS = 10, DEPTH = 75, LINEOFSIGHT = 7
	};
	PlayerInst(const CombatStats& stats, sprite_id sprite, int x, int y,
			bool local = true) :
			CombatGameInst(stats, sprite, 0, x, y, RADIUS, true, DEPTH), fieldofview(
					LINEOFSIGHT), local(local), money(0), lives(0), spellselect(
					0) {
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
	void queue_io_spell_and_attack_actions(GameState* gs, float dx, float dy);
	void queue_io_equipment_actions(GameState* gs);
	void queue_network_actions(GameState* gs);

	void perform_queued_actions(GameState* gs);
	void perform_action(GameState* gs, const GameAction& action);

	virtual bool within_field_of_view(const Pos& pos);

	int& spell_selected() {
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
	Weapon& weapon_type() {
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

private:
	/* Actions */
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
	bool didstep;
	bool local;

	int money, lives;
	int spellselect;
};

#endif /* PLAYERINST_H_ */
