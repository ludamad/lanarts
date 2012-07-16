/*
 * GameInstRef.h:
 *  Retains a reference to a GameInst object.
 *  This acts as a smart pointer to GameInst.
 *  Generally, only store obj_id and do object lookups wherever appropriate.
 *  This will help prevent any circular reference cases between objects.
 */

#include "../GameState.h"

#include "GameInst.h"
#include "GameInstRef.h"

static void __retain_ref(GameInst* inst) {
	if (inst) {
		inst->retain_reference();
	}
}
static void __free_ref(GameInst* inst) {
	if (inst) {
		inst->free_reference();
	}
}

GameInstRef::GameInstRef(GameInst* inst) :
		inst(inst) {
	__retain_ref(inst);
}

GameInstRef::GameInstRef(const GameInstRef& gref) :
		inst(gref.inst) {
	__retain_ref(inst);
}

GameInstRef::GameInstRef(GameState* gs, obj_id id) :
		inst(gs->get_instance(id)) {
	__retain_ref(inst);
}

GameInstRef::~GameInstRef() {
	__free_ref(inst);
}

void GameInstRef::operator =(const GameInstRef& ref) {
	__retain_ref(ref.inst);
	__free_ref(inst);
	inst = ref.inst;
}

