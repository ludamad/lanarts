/*
 * TeamRelations.cpp:
 *  Bridge to lua-defined team relationships.
 *  Fairly hardcoded & inefficient for now.
 */

#include <luawrap/luawrap.h>
#include "lua_api/lua_api.h"

#include "objects/CombatGameInst.h"

static lua_State* get_state(CombatGameInst* inst) {
	if (inst->lua_variables.empty()) {
		return NULL;
	}
	return inst->lua_variables.luastate();
}

static bool object_relations_callback(CombatGameInst* inst1, CombatGameInst* inst2, const char* func_name) {
	lua_State* L = get_state(inst1);
	if (L == NULL) {
		return false;
	}

	luawrap::globals(L)["import"].push();
	lua_pushstring(L, "lanarts.objects.relations");
	lua_call(L, 1, 1); // Get module

	lua_getfield(L, -1, func_name); // Get function
	lua_replace(L, -2); // Remove module
	return luawrap::call<bool>(L, inst1, inst2);
}

bool teamrelations_is_hostile(CombatGameInst* inst1, CombatGameInst* inst2) {
	return object_relations_callback(inst1, inst2, "is_hostile");
}

bool teamrelations_is_friendly(CombatGameInst* inst1, CombatGameInst* inst2) {
	return object_relations_callback(inst1, inst2, "is_friendly");
}

bool teamrelations_is_neutral(CombatGameInst* inst1, CombatGameInst* inst2) {
	return object_relations_callback(inst1, inst2, "is_neutral");
}
