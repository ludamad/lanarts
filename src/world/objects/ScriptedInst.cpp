/*
 * ScriptedInst.cpp:
 *  An object whose behaviour is entirely determined by lua callbacks.
 *  Uses for such objects:
 *   - Custom spell projectiles
 *   - Area of effect spell artifacts
 *   - Scripted animations
 */

#include <typeinfo>

#include "../../data/scriptobject_data.h"
#include "../../util/LuaValue.h"

#include "../GameState.h"

#include "ScriptedInst.h"

static inline ScriptObjectEntry& __S(scriptobj_id script_obj_id) {
	return game_scriptobject_data.at(script_obj_id);
}

ScriptedInst::ScriptedInst(scriptobj_id script_obj, int x, int y) :
		GameInst(__S(script_obj).radius, x, y), script_obj_id(script_obj) {
}

ScriptedInst::~ScriptedInst() {
}

void ScriptedInst::init(GameState* gs) {
	lua_State* L = gs->get_luastate();
	ScriptObjectEntry& scr_obj = script_object();
	//Init the object
	lua_gameinst_callback(L, scr_obj.init_event, this);
}

void ScriptedInst::step(GameState* gs) {
	lua_State* L = gs->get_luastate();
	ScriptObjectEntry& scr_obj = script_object();
	//Step the object
	lua_gameinst_callback(L, scr_obj.step_event, this);
}

void ScriptedInst::draw(GameState* gs) {
	ScriptObjectEntry& scr_obj = script_object();
	gl_draw_sprite(scr_obj.sprite, x, y);
}

void ScriptedInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ScriptedInst*) inst = *this;
}

ScriptedInst* ScriptedInst::clone() const {
	return new ScriptedInst(*this);
}

ScriptObjectEntry& ScriptedInst::script_object() {
	return __S(script_obj_id);
}

