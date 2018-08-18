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

extern ResourceDataSet<SpellEntry> game_spell_data;
extern ResourceDataSet<EffectEntry> game_effect_data;
extern ResourceDataSet<ItemEntry*> game_item_data;

void lapi_data_create_spell(const LuaStackValue& table) {
	SpellEntry entry;
	int idx = game_spell_data.size();
	entry.init(idx, table);
	game_spell_data.new_entry(entry.name, entry, table);
}


static EffectEntry parse_effect(const LuaStackValue& table) {
    using namespace luawrap;
    lua_State* L = table.luastate();
    EffectEntry entry;
    entry.name = table["name"].to_str();
    entry.category = defaulted(table, "category", std::string());

    entry.raw_lua_object = table;

    entry.init_func = table["init_func"];
    entry.draw_func = table["draw_func"];
    entry.stat_func = table["stat_func"];
    entry.attack_stat_func = table["attack_stat_func"];
    entry.step_func = table["step_func"];

    entry.console_draw_func = table["console_draw_func"];
    entry.remove_func = table["remove_func"];
    entry.remove_derived_func = table["remove_derived_func"];
    entry.apply_derived_func = table["apply_derived_func"];
    entry.apply_buff_func = table["apply_buff_func"];

    // Configurability options for equipment / spell granted effects:
    entry.on_melee_func = table["on_melee_func"];
    entry.on_projectile_func = table["on_projectile_func"];
    entry.on_receive_melee_func = table["on_receive_melee_func"];
    entry.on_equip_func = table["on_equip_func"];
    entry.on_identify_func = table["on_identify_func"];
    entry.on_uncurse_func = table["on_uncurse_func"];
    entry.on_gain_spell_func = table["on_gain_spell_func"];

    entry.attack_stat_func = table["attack_stat_func"];
    entry.step_func = table["step_func"];

    entry.allowed_actions.can_use_stairs = defaulted(table, "can_use_stairs", true);
    entry.allowed_actions.can_use_rest = defaulted(table, "can_use_rest",
            true);
    entry.allowed_actions.can_use_spells = defaulted(table, "can_use_spells",
            true);
    entry.allowed_actions.can_use_items = defaulted(table, "can_use_items", true);
    entry.allowed_actions.can_use_weapons = defaulted(table, "can_use_weapons", true);

    entry.effected_colour = defaulted(table, "effected_colour", Colour());
    if (!table["effected_sprite"].isnil()) {
        entry.effected_sprite = res::sprite_id(table["effected_sprite"].to_str());
    }

    entry.additive_duration = defaulted(table, "additive_duration", false);
    entry.fade_out = defaulted(table, "fade_out", 5);
    return entry;
}

void lapi_data_create_effect(const LuaStackValue& table) {
    EffectEntry entry = parse_effect(table);
    game_effect_data.new_entry(entry.name, entry, table);
}


static ProjectileEntry* parse_projectile(const LuaStackValue& table) {
    ProjectileEntry* entry = new ProjectileEntry;
    entry->init(game_item_data.size(), table);
    return entry;
}

void lapi_data_create_projectile(const LuaStackValue& table) {
    ProjectileEntry* entry = parse_projectile(table);
    table["__method"] = "projectile_create"; 
    game_item_data.new_entry(entry->name, entry, table);
}
