#include "stats/items/ItemEntry.h"
#include "lua_api/lua_api.h"

namespace lua_api {
    void register_lua_core_GameData(lua_State* L) {
        LuaValue module = register_lua_submodule(L, "core.GameData");
        module["item_definer"] = [=](const char* type) -> LuaValue {
            ItemEntry* entry = game_item_data.get(type);
            return entry->raw;
        };
    }
}