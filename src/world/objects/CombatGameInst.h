/*
 * CombatGameInst.h:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#ifndef COMBATGAMEINST_H_
#define COMBATGAMEINST_H_

#include "../../gamestats/combat_stats.h"
#include "../../gamestats/stats.h"

#include "../../pathfind/pathfind.h"

#include "GameInst.h"

struct AttackStats;

struct EntityBehaviour {
	int simulation_id; // For collision-avoidance
};

class CombatGameInst: public GameInst {
public:
	CombatGameInst(const CombatStats& base_stats, sprite_id sprite, int teamid,
			int x, int y, int radius, bool solid = true, int depth = 0) :
			GameInst(x, y, radius, solid, depth), rx(x), ry(y), vx(0), vy(0), is_resting(
					false), teamid(teamid), spriteid(sprite), base_stats(
					base_stats) {
	}
	virtual ~CombatGameInst() {
	}

	virtual void die(GameState* gs) = 0;
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void update_field_of_view();
	virtual bool within_field_of_view(const Pos& pos) =0;

	void attempt_move_to_position(GameState* gs, float& newx, float& newy);
	//update based on rounding of true float
	void update_position();
	void update_position(float newx, float newy);

	bool damage(GameState* gs, int dmg);
	bool melee_attack(GameState* gs, CombatGameInst* inst, const Weapon& projectile);
	bool projectile_attack(GameState* gs, CombatGameInst* inst, const Weapon& weapon, const Projectile& projectile);
	//bool spell_attack ...
	bool attack(GameState* gs, CombatGameInst* inst, const AttackStats& attack);

	void equip(item_id item, int amnt = 1);

	CombatStats& stats();
	CoreStats& core_stats();
	ClassStats& class_stats();

	/* With attack data */
	EffectiveAttackStats effective_atk_stats(MTwist& mt, const AttackStats& attack);
	/* Damage, power, cooldown not complete without attack specified: */
	EffectiveStats& effective_stats();

	CooldownStats& cooldowns();
	SpellsKnown& spells_known();
	EffectStats& effects();
	Inventory& inventory();
	Equipment& equipment();

	team_id& team();


	float vx, vy;
protected:
	float rx, ry;
	bool is_resting;
	team_id teamid;
	sprite_id spriteid;
//	PathInfo path;
private:
	void ensure_valid_direction(GameState* gs, float speed);

	CombatStats base_stats;
	EffectiveStats estats;
};

#endif /* COMBATGAMEINST_H_ */
