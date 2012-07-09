/*
 * ScriptedInst.cpp:
 *  An object whose behaviour is entirely determined by lua callbacks.
 *  Uses for such objects:
 *   - Custom spell projectiles
 *   - Area of effect spell artifacts
 *   - Scripted animations
 */

#include "ScriptedInst.h"

ScriptedInst::ScriptedInst(scriptobject_id script_obj, int radius, int x, int y) :
		GameInst(radius, x, y), script_obj(script_obj) {
}

ScriptedInst::~ScriptedInst() {
}

void ScriptedInst::init(GameState *gs) {
}

void ScriptedInst::step(GameState *gs) {
}

void ScriptedInst::draw(GameState *gs) {
}

