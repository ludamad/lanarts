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

#include "lua_api/lua_api.h"

#include "objects/CombatGameInst.h"
#include "objects/GameInst.h"
#include "objects/PlayerIOActions.h"

const int REST_COOLDOWN = 200;
// TODO: Now used for all 'major characters', i.e, players, their allies, and bosses
const int PLAYER_PATHING_RADIUS = 500;

/* Statistics that do not affect gameplay but are kept for scoring */
struct PlayerScoreStats {
    int deaths = 0, kills = 0, deepest_floor = 0;
};

struct PlayerExploreState {
    Pos move_xy;
    int time_out = 0; // move_xy is invalid if time_out <= 0
    void set_move(Pos move_xy, int time_out) {
        this->move_xy = move_xy;
        if (move_xy == Pos()) {
            time_out = 0;
        } else {
            time_out = time_out;
        }
    }
    void step() {
        if (time_out <= 0) {
            return;
        }
        time_out -= 1;
    }
};

struct PlayerDataEntry;

/* The player object, one exists globally per player */
class PlayerInst: public CombatGameInst {
public:
    enum {
        RADIUS = 10, DEPTH = 75
    };
    PlayerInst(const CombatStats& stats, sprite_id sprite, Pos xy, team_id team, bool local);
    PlayerInst() {
        field_of_view = NULL;
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
    virtual bool is_major_character() {
        return true;
    }

    virtual std::vector<StatusEffect> base_status_effects(GameState* gs);
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

    money_t& gold(GameState* gs);

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

    bool is_focus_player(GameState *gs) const;
    bool is_local_player() const {
        return local;
    }
    void set_local_player(bool islocal) {
        local = islocal;
    }

    bool& actions_set() {
        return actions_set_for_turn;
    }

    // Ghosts are players that have died. They may move around the current level, 
    // but do not otherwise interact with the game.
    // They respawn when a player (included the player that had died) uses a revive elixir near them.
    bool melee_attack(GameState* gs, CombatGameInst* inst,
                    const Item& weapon, bool ignore_cooldowns = false, 
                    float damage_multiplier = 1.0f);
    PosF& last_moved_direction() {
        return _last_moved_direction;
    }

    // Are we allowed to rest, and can we regenerate mp, hp, or spell cooldown?
    bool can_benefit_from_rest();
    Pos direction_towards_unexplored(GameState* gs, bool* finished = NULL);
    PlayerIOActions& input_source() {
        return io_value;
    }

    // Used by cheat/debug codes
    void pickup_item(GameState* gs, const GameAction& action);
    // Used by lua-based item management
    void sell_item(GameState* gs, const GameAction& action);
private:
    ///////////////////////////
    // ** Private methods ** //
    ///////////////////////////
    void enqueue_io_movement_actions(GameState* gs, float& dx, float& dy);
    bool enqueue_io_spell_actions(GameState* gs, bool* fallback_to_melee);
    bool enqueue_io_spell_and_attack_actions(GameState* gs, float dx, float dy);
    void enqueue_io_equipment_actions(GameState* gs, bool do_stop_action);
    bool enqueue_not_enough_mana_actions(GameState* gs);

    //Game action events
    void _use_move(GameState *gs, const GameAction &action);
    void _use_weapon(GameState *gs, const GameAction &action);
    void _use_dngn_portal(GameState *gs, const GameAction &action);
    void _use_spell(GameState *gs, const GameAction &action);
    void _channel_spell(GameState *gs, const GameAction &action);
    void _use_rest(GameState *gs, const GameAction &action);
    void _use_item(GameState *gs, const GameAction &action);

    //Either finds new target or shifts target
    void shift_autotarget(GameState* gs);

    void drop_item(GameState* gs, const GameAction& action);
    void reposition_item(GameState* gs, const GameAction& action);
    void purchase_from_store(GameState* gs, const GameAction& action);

    ///////////////////
    // ** Members ** //
    ///////////////////
    PlayerScoreStats _score_stats;

    bool actions_set_for_turn= false;
    ActionQueue queued_actions;
    // Used when eg run out of projectiles.
    // We will switch to another weapon type but want to
    // switch back the moment we pick up a projectile
    std::string last_chosen_weaponclass;

    // NB: local to spell select is assumed to be a POD region by serialize/deserialize
    bool local = false, moving = false;
    int autouse_mana_potion_try_count = 0;
    PosF _last_moved_direction = {0, -1}; // Never 0,0
    int previous_spell_cast = -1, spellselect = 0;
    PlayerExploreState explore_state;
    PlayerIOActions io_value;
};

bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position);

#endif /* PLAYERINST_H_ */
