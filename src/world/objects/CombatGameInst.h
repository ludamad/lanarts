/*
 * CombatGameInst.h:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#ifndef COMBATGAMEINST_H_
#define COMBATGAMEINST_H_

#include "../../gamestats/stats.h"

#include "../../pathfind/pathfind.h"

#include "GameInst.h"


struct EntityBehaviour {
	int simulation_id; // For collision-avoidance

};

class CombatGameInst: public GameInst {
public:
	CombatGameInst(int x, int y, int radius, bool solid = true, int depth = 0) :
			GameInst(x, y, radius, solid, depth) {
	}
	/*  GameInst members */
	virtual void step(GameState* gs);

	/* Querying */
	virtual void update_field_of_view();
	virtual bool within_field_of_view(const Pos& pos) = 0;

	bool damage(GameState* gs, int dmg);

	/* Getters */
	CombatStats& base_stats();
	EffectiveStats& effective_stats();
protected:
	PathInfo path;
	CombatStats stats;
private:
	EffectiveStats estats;
};





#endif /* COMBATGAMEINST_H_ */
