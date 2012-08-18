/*
 * GameInstRef.h:
 *  Retains a reference to a GameInst object.
 *  This acts as a smart pointer to GameInst.
 *  Generally, only store obj_id and do object lookups wherever appropriate.
 *  This will help prevent any circular reference cases between objects.
 */

#ifndef GAMEINSTREF_H_
#define GAMEINSTREF_H_

#include "../lanarts_defines.h"

class GameState;
class GameInst;

class GameInstRef {
public:
	GameInstRef(GameInst* inst = NULL);
	GameInstRef(const GameInstRef& gref);
	GameInstRef(GameState* gs, obj_id id);
	~GameInstRef();

	GameInst* get_instance() const {
		return inst;
	}
	void operator=(const GameInstRef& ref);
	void operator=(GameInst* ref);
	bool empty() const {
		return inst == NULL;
	}
	GameInst* operator->() const {
		return inst;
	}
private:
	GameInst* inst;
};

#endif /* GAMEINSTREF_H_ */
