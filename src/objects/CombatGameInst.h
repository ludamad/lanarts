/*
 * CombatGameInst.h:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#ifndef COMBATGAMEINST_H_
#define COMBATGAMEINST_H_

#include "collision_avoidance/CollisionAvoidance.h"

#include "pathfind/FloodFillPaths.h"
#include "stats/combat_stats.h"
#include "fov/fov.h"

#include "stats/stats.h"

#include "GameInst.h"

struct AttackStats;

struct CollisionAvoidanceParameters {
	simul_id collision_simulation_id;
	int avoidance_radius;
};

class CombatGameInst: public GameInst {
public:
	CombatGameInst(const CombatStats& base_stats, sprite_id sprite, Pos xy, team_id team, float radius, bool solid =
					true, int depth = 0) :
			GameInst(xy.x, xy.y, radius, solid, depth), rx(xy.x), ry(xy.y), vx(0), vy(0), is_resting(
					false), sprite(sprite), simulation_id(
					-1), current_target(NONE), base_stats(base_stats), team(team) {
	}
	CombatGameInst() :
			GameInst(0, 0, 0) {
	}
    virtual ~CombatGameInst();

	virtual void die(GameState* gs) = 0;
    virtual void init(GameState* gs);
    virtual void deinit(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs, float frame = 0, float alpha = 1);
	virtual void post_draw(GameState* gs);
	virtual void update_field_of_view(GameState* gs);
	virtual bool within_field_of_view(const Pos& pos) =0;

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

	virtual unsigned int integrity_hash();

	PosF attempt_move_to_position(GameState* gs, const PosF& new_xy);
	//update based on rounding of true float
	void update_position();
	virtual void update_position(float newx, float newy);

	virtual bool damage(GameState* gs, float dmg, CombatGameInst* attacker);
	bool damage(GameState* gs, const EffectiveAttackStats& attack, CombatGameInst* attacker = NULL, float* final_dmg = NULL);
	virtual bool melee_attack(GameState* gs, CombatGameInst* inst,
			const Weapon& weapon, bool ignore_cooldowns = false, 
                        float damage_multiplier = 1.0f);
	bool projectile_attack(GameState* gs, CombatGameInst* inst,
			const Weapon& weapon, const Projectile& projectile);
	bool attack(GameState* gs, CombatGameInst* inst, const AttackStats& attack);

	virtual void signal_attacked_successfully() {
	}

	virtual void signal_killed_enemy() {
	}

	CombatStats& stats();
	CoreStats& core_stats();
	ClassStats& class_stats();

        Pos direction_towards(GameState* gs, GameInst* inst);
        Pos direction_towards_enemy(GameState* gs);
        Pos direction_towards_object(GameState* gs, col_filterf filter);
        Pos direction_towards_ally_player(GameState* gs);

	/* With attack data */
	EffectiveAttackStats effective_atk_stats(MTwist& mt,
			const AttackStats& attack);
	/* Damage, power, cooldown not complete without attack specified: */
	EffectiveStats& effective_stats();

	CooldownStats& cooldowns();
	SpellsKnown& spells_known();
	Inventory& inventory();
	EquipmentStats& equipment();

	void gain_xp_from(GameState* gs, CombatGameInst* inst, float dx = -5, float dy = -5);
	sprite_id get_sprite() const {
		return sprite;
	}

        bool& is_ghost() {
            return _is_ghost;
        }
	simul_id& collision_simulation_id();

	obj_id& target() {
		return current_target;
	}
	bool has_paths_data() {
	    return _paths_to_object != NULL;
	}
	FloodFillPaths& paths_to_object() {
	    LANARTS_ASSERT(has_paths_data());
	    return *_paths_to_object;
	}
	int& xpworth(){
		return xpgain;
	}

	void use_projectile_spell(GameState* gs, SpellEntry& spl_entry,
							  const Projectile& projectile, const Pos& target);
	void use_spell(GameState* gs, SpellEntry& spl_entry, const Pos& target, GameInst* target_object = nullptr);
//members
public:
// <PURE DATA REGION see (de)serialize>
	team_id team = NONE;
	float vx = -1, vy = -1;
	float rx = -1, ry = -1;
	bool is_resting = false;
	int xpgain = 0;
    int vision_radius = 0;
protected:
	sprite_id sprite = NONE;
	simul_id simulation_id = NONE;
        bool _is_ghost = false;
	obj_id current_target = NONE;
// </PURE DATA REGION see (de)serialize>
public:
    // field_of_view:
    //   Used for decisions about whether one object sees another.
    //   This is used for all allies of the player, although most allies do not provide the player
    //   with their full field of view.
    //   This is used to decide if a major team member sees an enemy, but other NPCs have more primitive sight code.
    fov* field_of_view = NULL;
    // _paths_to_object:
    //   Used for decisions about pathing to the object.
    //   For player team members this is updated every step and perfect pathing towards the object is used.
    //   For minor team members (every other NPC) this is not used.
    FloodFillPaths* _paths_to_object = NULL;
private:
	CombatStats base_stats;
	EffectiveStats estats;
};
#endif /* COMBATGAMEINST_H_ */
