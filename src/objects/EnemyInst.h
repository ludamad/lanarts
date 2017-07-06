/*
 * EnemyInst.h:
 *  Represents an AI controlled combat entity
 */

#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "stats/stats.h"
#include "objects/CombatGameInst.h"
#include "objects/GameInst.h"

#include "objects/EnemyBehaviour.h"

#include "objects/EnemyEntry.h"

constexpr int MONSTER_TEAM = 1;
class EnemyInst : public CombatGameInst {
public:
	EnemyInst(int enemytype, int x, int y, int team = MONSTER_TEAM);
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

        virtual std::vector<StatusEffect> base_status_effects(GameState* gs);

	EnemyBehaviour& behaviour() { return eb; }
	EnemyEntry& etype();
	int enemy_type() {
            return enemytype;
        }
    virtual bool is_major_character() {
        return false; // TODO Eventually have major enemies.
        // Bosses, perhaps, might want exact pathing towards them.
    }

private:
	bool seen;
	EnemyBehaviour eb;
    int enemytype;
	int enemy_regen_cooloff;
};


#endif /* ENEMYINST_H_ */
