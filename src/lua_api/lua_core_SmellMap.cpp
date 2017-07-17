/*
 * lua_core_SmellMap.cpp
 * Bindings for gamestate/SmellMap.h
 */

#include <SDL.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_api.h"

#include "lua_api/lua_api.h"

#include "gamestate/GameState.h"
#include "gamestate/SmellMap.h"

GameMapState* mapstate(LuaStackValue map_obj, bool defaulted = false);

typedef smartptr<SmellMap> SmellMapPtr;

LuaValue lua_smellmapmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "SmellMap");
	LuaValue methods = luameta_constants(meta);
	methods["step"] = [=](SmellMapPtr ptr) {
            ptr->step();
        };
	methods["add_smell"] = [=](SmellMapPtr ptr, CombatGameInst* inst, float smell) {
            ptr->add_smell(inst, smell);
        };
	methods["towards_least_smell"] = [=](SmellMapPtr ptr, CombatGameInst* inst) {
            return ptr->towards_least_smell(inst);
        }
	methods["towards_most_smell"] = [=](SmellMapPtr ptr, CombatGameInst* inst) {
            return ptr->towards_most_smell(inst);
        }
	luameta_gc<SmellMapPtr>(meta);
	return meta;
}

namespace lua_api {
	void register_lua_core_SmellMap(lua_State* L) {
		LuaValue smellmap = register_lua_submodule(L, "core.SmellMap");
		luawrap::install_userdata_type<SmellMapPtr, &lua_smellmapmetatable>();
		smellmap["create"] = [=](LuaStackValue map_obj) {
                    auto* map = mapstate(map_obj);
                    return SmellMapPtr(new SmellMap {map->tiles()});
                };
	}
}
