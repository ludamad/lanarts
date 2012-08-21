/*
 * EnemyInst.h:
 *  Represents an AI controlled combat entity
 */

#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "../CombatGameInst.h"
#include "../GameInst.h"
#include "EnemyBehaviour.h"

#include "../../stats/stats.h"

#include "enemy_data.h"

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

	virtual void signal_attacked_successfully();
	virtual void signal_was_damaged();

	virtual unsigned int integrity_hash();

	virtual bool within_field_of_view(const Pos& pos);

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

	EnemyBehaviour& behaviour() { return eb; }
	EnemyEntry& etype();
	int xpworth(){
		return xpgain;
	}
protected:
	bool seen;
	int enemytype;
	EnemyBehaviour eb;
	int xpgain;
};


#endif /* ENEMYINST_H_ */
