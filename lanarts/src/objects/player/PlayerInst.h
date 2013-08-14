/* PlayerInst.h:
 *  Represents a player of the game, in a networked game there can be many, only one of
 *  which is the 'local' player.
 *  See player_impl/ for member implementations.
 */

#ifndef PLAYERINST_H_
#define PLAYERINST_H_

#include <deque>

#include "draw/SpriteEntry.h"

#include "fov/fov.h"

#include "gamestate/ActionQueue.h"

#include "gamestate/GameAction.h"

#include "gamestate/GameMapState.h"
#include "pathfind/FloodFillPaths.h"

#include "lua_api/lua_newapi.h"

#include "../CombatGameInst.h"
#include "../GameInst.h"

const int REST_COOLDOWN = 200;
const int PLAYER_PATHING_RADIUS = 500;

/* Statistics that do not affect gameplay but are kept for scoring */
struct PlayerScoreStats {
	PlayerScoreStats() {
		deaths = 0;
		kills = 0;
		deepest_floor = 0;
	}
	int deaths, kills, deepest_floor;
};

struct PlayerDataEntry;

/* The player object, one exists globally per player */
class PlayerInst: public CombatGameInst {
public:
	enum {
		RADIUS = 10, DEPTH = 75, LINEOFSIGHT = 7
	};
	PlayerInst(const CombatStats& stats, sprite_id sprite, int x, int y,
			bool local = true);
	PlayerInst() :
			fieldofview(LINEOFSIGHT), local(false) {
	}

	virtual ~PlayerInst();
	virtual void init(GameState* gs);
	virtual void die(GameState* gs);
	virtual void deinit(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

	virtual PlayerInst* clone() const;
	void gain_xp(GameState* gs, int xp);

	void enqueue_io_actions(GameState* gs);
	void enqueue_actions(const ActionQueue& queue);

	void perform_queued_actions(GameState* gs);
	void perform_action(GameState* gs, const GameAction& action);

	//Field of view
	virtual void update_field_of_view(GameState* gs);
	virtual bool within_field_of_view(const Pos& pos);

	virtual void signal_killed_enemy() {
		_score_stats.kills++;
	}

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

	const PlayerScoreStats& score_stats() const {
		return _score_stats;
	}

	Weapon weapon() {
		return equipment().weapon();
	}

	Projectile projectile() {
		return equipment().projectile();
	}

	PlayerDataEntry& player_entry(GameState* gs) const;

	bool is_local_player() {
		return local;
	}
	void set_local_player(bool islocal) {
		local = islocal;
	}

	fov& field_of_view() {
		return fieldofview;
	}
	FloodFillPaths& path_to_player() {
		return _path_to_player;
	}

	bool& actions_set() {
		return actions_set_for_turn;
	}
private:

	void enqueue_io_movement_actions(GameState* gs, int& dx, int& dy);
	bool enqueue_io_spell_actions(GameState* gs, bool* fallback_to_melee);
	bool enqueue_io_spell_and_attack_actions(GameState* gs, float dx, float dy);
	void enqueue_io_equipment_actions(GameState* gs, bool do_stop_action);
	bool enqueue_not_enough_mana_actions(GameState* gs);

	//Game action events
	void use_move(GameState* gs, const GameAction& action);
	void use_weapon(GameState* gs, const GameAction& action);
	void use_dngn_portal(GameState* gs, const GameAction& action);
	void use_spell(GameState* gs, const GameAction& action);
	void use_rest(GameState* gs, const GameAction& action);
	void use_item(GameState* gs, const GameAction& action);

	//Either finds new target or shifts target
	void shift_autotarget(GameState* gs);

	void pickup_item(GameState* gs, const GameAction& action);
	void drop_item(GameState* gs, const GameAction& action);
	void reposition_item(GameState* gs, const GameAction& action);
	void purchase_from_store(GameState* gs, const GameAction& action);

	PlayerScoreStats _score_stats;

	bool actions_set_for_turn;
	ActionQueue queued_actions;
	FloodFillPaths _path_to_player;
	fov fieldofview;

	// Used when eg run out of projectiles.
	// We will switch to another weapon type but want to
	// switch back the moment we pick up a projectile
	std::string last_chosen_weaponclass;

	// NB: local to spell select is assumed to be a POD region by serialize/deserialize
	bool local, moving;
	int autouse_mana_potion_try_count;
	int previous_spellselect, spellselect;
};

bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position);

#endif /* PLAYERINST_H_ */
