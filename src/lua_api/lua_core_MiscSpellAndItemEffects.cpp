/*
 * lua_core_MiscSpellAndItemEffects.cpp
 */

#include <luawrap/luawrap.h>

#include <ldungeon_gen/map_misc_ops.h>

#include "objects/PlayerInst.h"
#include "objects/StoreInst.h"
#include "objects/FeatureInst.h"
#include "objects/ItemInst.h"
#include "gamestate/GameState.h"

#include "lua_api.h"

// Reveal a map completely:
static void magic_map_effect(LuaStackValue linst) {
    PlayerInst* inst = linst.as<PlayerInst*>();
    auto* gs = lua_api::gamestate(linst);
    auto* level = gs->get_level(inst->current_floor);
    auto& tiles = level->tiles();
    auto seen_map = tiles.previously_seen_map();
    auto source_map = level->source_map();
    auto area = BBox({0,0}, source_map->size());
    ldungeon_gen::Selector unsolid_selector {0, ldungeon_gen::FLAG_SOLID};
    ldungeon_gen::Operator mark {ldungeon_gen::FLAG_RESERVED2};
    ldungeon_gen::Selector marked {ldungeon_gen::FLAG_RESERVED2};
    ldungeon_gen::area_fully_connected(*source_map, area, unsolid_selector, mark, marked);
    FOR_EACH_BBOX(area, x, y) {
        if ((*source_map)[{x,y}].matches(marked)) {
            for (int dy = -1; dy <= +1; dy++) {
                for (int dx = -1; dx <= +1; dx++) {
                    (*seen_map)[{x+dx, y+dy}] = true;
                }
            }
        }
    }
    for (auto* inst : level->game_inst_set().to_vector()) {
        ItemInst* item = dynamic_cast<ItemInst*>(inst);
        if (item) {
            item->set_as_seen();
        }
        FeatureInst* feature = dynamic_cast<FeatureInst*>(inst);
        if (feature) {
            feature->set_as_seen();
        }
        StoreInst* store = dynamic_cast<StoreInst*>(inst);
        if (store) {
            store->set_as_seen();
        }
    }
}

namespace lua_api {
	void register_lua_core_MiscSpellAndItemEffects(lua_State* L) {
		// Mouse API
		LuaValue module = lua_api::register_lua_submodule(L, "core.MiscSpellAndItemEffects");
		module["magic_map_effect"].bind_function(magic_map_effect);
//		mouse.getters["mouse_xy"].bind_function(mouse_xy);
//		mouse.getters["mouse_left_held"].bind_function(mouse_left_held);
//		mouse.getters["mouse_left_pressed"].bind_function(mouse_left_pressed);
//		mouse.getters["mouse_right_held"].bind_function(mouse_right_held);
//		mouse.getters["mouse_right_pressed"].bind_function(mouse_right_pressed);
	}
}
