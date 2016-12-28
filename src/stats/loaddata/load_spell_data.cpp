/*
 * load_spell_data.cpp:
 *  Implements spell loading routines from YAML datafiles
 */
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "draw/SpriteEntry.h"
#include "data/game_data.h"

#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"

#include "load_stats.h"

using namespace std;

extern std::vector<SpellEntry> game_spell_data;
extern std::vector<EffectEntry> game_effect_data;
extern std::vector<ItemEntry*> game_item_data;


static LuaValue spell_table;
static void lapi_data_create_spell(const LuaStackValue& table) {
	SpellEntry entry;
	int idx = game_spell_data.size();
	entry.init(idx, table);
	spell_table[idx+1] = table;
	game_spell_data.push_back(entry);
}


static EffectEntry parse_effect(const LuaStackValue& table) {
    using namespace luawrap;
    lua_State* L = table.luastate();
    EffectEntry entry;
    entry.name = table["name"].to_str();
    entry.init_func.initialize(table["init_func"]);
    entry.stat_func.initialize(table["stat_func"]);
    entry.finish_func.initialize(table["finish_func"]);
    entry.attack_stat_func.initialize(table["attack_stat_func"]);
    entry.step_func.initialize(table["step_func"]);

    entry.allowed_actions.can_use_stairs = defaulted(table, "can_use_stairs", true);
    entry.allowed_actions.can_use_rest = defaulted(table, "can_use_rest",
            true);
    entry.allowed_actions.can_use_spells = defaulted(table, "can_use_spells",
            true);
    entry.allowed_actions.can_use_items = defaulted(table, "can_use_items",
            true);

    entry.effected_colour = defaulted(table, "effected_colour", Colour());
    entry.effected_sprite = res::sprite_id(table["effected_sprite"].to_str());
    entry.additive_duration = defaulted(table, "additive_duration", false);
    entry.fade_out = defaulted(table, "fade_out", 100);
    entry.init(L);
    return entry;
}

static void lapi_data_create_effect(const LuaStackValue& table) {
    game_effect_data.push_back(parse_effect(table));
}


static ProjectileEntry* parse_projectile(const LuaStackValue& table) {
    ProjectileEntry* entry;
    entry->parse_lua_table(table);
    return entry;
}

static void lapi_data_create_projectile(const LuaStackValue& table) {
    game_item_data.push_back(parse_projectile(table));
}

LuaValue load_spell_data(lua_State* L, const FilenameList& filenames) {
	game_spell_data.clear();

	spell_table = LuaValue(L);
	spell_table.newtable();

	LuaValue data = luawrap::ensure_table(luawrap::globals(L)["Data"]);
    data["spell_create"].bind_function(lapi_data_create_spell);
    data["effect_create"].bind_function(lapi_data_create_effect);
    data["projectile_create"].bind_function(lapi_data_create_projectile);
	luawrap::dofile(L, "spells/Spells.lua");

	return spell_table;
}
