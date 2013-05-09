/*
 * EnemyInst.h:
 *  Represents an AI controlled combat entity
 */

#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "stats/stats.h"
#include "../CombatGameInst.h"
#include "../GameInst.h"
#include "EnemyAIState.h"

#include "EnemyBehaviour.h"

#include "EnemyEntry.h"

class EnemyInst : public CombatGameInst {
public:
	EnemyInst(int enemytype, int x, int y, int teamid, int mobid = -1);
	virtual ~EnemyInst();

	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void die(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual EnemyInst* clone() const;

	virtual bool damage(GameState* gs, int dmg);

	virtual void signal_attacked_successfully();

	virtual unsigned int integrity_hash();

	virtual bool within_field_of_view(const Pos& pos);

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

	EnemyBehaviour& behaviour() { return eb; }
	EnemyEntry& etype();
	int xpworth(){
		return xpgain;
	}

private:
	bool seen;
	int enemytype;
	EnemyBehaviour eb;
	int enemy_regen_cooloff;
//	EnemyAIState ai_state;
	int xpgain;
};


#endif /* ENEMYINST_H_ */
