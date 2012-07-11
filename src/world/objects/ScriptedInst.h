/*
 * ScriptedInst.h:
 *  An object whose behaviour is entirely determined by lua callbacks.
 *  Uses for such objects:
 *   - Custom spell projectiles
 *   - Area of effect spell artifacts
 *   - Scripted animations
 */

#ifndef SCRIPTEDINST_H_
#define SCRIPTEDINST_H_

#include "../../util/game_basic_structs.h"

#include "GameInst.h"

struct ScriptObjectEntry;

class ScriptedInst: public GameInst {
public:
	ScriptedInst(scriptobj_id script_obj, int x, int y);
	virtual ~ScriptedInst();

	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);

	virtual void copy_to(GameInst* inst) const;
	virtual ScriptedInst* clone() const;

	ScriptObjectEntry& script_object();
protected:
	scriptobj_id script_obj_id;
};

#endif /* SCRIPTEDINST_H_ */
