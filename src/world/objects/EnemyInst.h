#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"
#include "EnemyBehaviour.h"

#include "../../gamestats/Stats.h"

#include "../../data/enemy_data.h"

class EnemyInst : public GameInst {
public:
	EnemyInst(int enemytype, int x, int y);
	virtual ~EnemyInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual EnemyInst* clone() const;


	void attack(GameState* gs, GameInst* inst, bool ranged);
	Stats& stats() { return stat; }
	EnemyBehaviour& behaviour() { return eb; }
	EnemyEntry* etype();
	int draw_radius() {
		return real_radius;
	}
	int xpworth(){
		return xpgain;
	}
	bool hurt(GameState* gs, int hp);
public:
	float rx, ry;
protected:
	float real_radius;
	bool seen;
	int enemytype;
	EnemyBehaviour eb;
	int xpgain;
    Stats stat;
};


#endif /* ENEMYINST_H_ */
