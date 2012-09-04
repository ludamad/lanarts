#include <fstream>

extern "C" {
#include <lua/lua.h>
}

#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../item_data.h"

#include "load_stats.h"

using namespace std;

void _load_armour_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	_ArmourEntry entry;
	entry.name = parse_str(node["name"]);
	entry.description = parse_defaulted(node, "description", std::string());
	entry.shop_cost = parse_defaulted(node, "shop_cost", Range());
	entry.damage_reduction = parse_defaulted(node, "reduction",
			CoreStatMultiplier());
	entry.resistance = parse_defaulted(node, "resistance",
			CoreStatMultiplier());
	entry.magic_reduction = parse_defaulted(node, "magic_reduction",
			CoreStatMultiplier());
	entry.magic_resistance = parse_defaulted(node, "magic_resistance",
			CoreStatMultiplier());
	entry.spell_cooldown_multiplier = parse_defaulted(node,
			"spell_cooldown_multiplier", 1.0f);
	entry.item_sprite = parse_sprite_number(node, "spr_item");

	game_armour_data.push_back(entry);

	value->table_set_yaml(L, entry.name, node);
	value->table_push_value(L, entry.name);
	lua_pushstring(L, "armour");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);
}

void _load_armour_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	game_armour_data.clear();
	load_data_impl_template(filenames, "armours", _load_armour_callbackf, L,
			itemtable);
}

void _load_armour_item_entries() {
	const int default_radius = 11;

	//Create items from armours
	for (int i = 0; i < game_armour_data.size(); i++) {
		_ArmourEntry& entry = game_armour_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		_game_item_data.push_back(
				_ItemEntry(entry.name, entry.description, "", default_radius,
						entry.item_sprite, "equip", "", false, entry.shop_cost,
						_ItemEntry::ALWAYS_KNOWN, _ItemEntry::ARMOUR, i));
	}
}


void parse_equipment_entry(const YAML::Node& n, EquipmentEntry& entry) {
	parse_item_entry(n, entry);
	entry.stackable = false;
//	entry.effect_modifiers; TODO
	entry.use_action = LuaAction(LuaValue("equip"));
	entry.stat_modifiers = parse_stat_modifiers(n);
	entry.cooldown_modifiers = parse_cooldown_modifiers(n);
	entry.type = EquipmentEntry::ARMOUR;
//  entry.type; TODO
}

void load_equipment_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	EquipmentEntry* entry = new EquipmentEntry;
	parse_equipment_entry(node, *entry);

	game_item_data.push_back(entry);
	/* Lua loading code */
	value->table_set_yaml(L, entry->name, node);
	value->table_push_value(L, entry->name);
	lua_pushstring(L, "armour");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);

}

void load_equipment_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	load_data_impl_template(filenames, "armours", load_equipment_callbackf, L,
			itemtable);
}
