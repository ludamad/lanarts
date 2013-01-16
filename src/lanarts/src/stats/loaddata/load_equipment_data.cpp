#include <fstream>

#include <lua.hpp>

#include "lua_api/lua_yaml.h"
#include "data/game_data.h"
#include "data/yaml_util.h"

#include "../items/ItemEntry.h"

#include "load_stats.h"

using namespace std;

void parse_equipment_entry(lua_State* L, const YAML::Node& n, EquipmentEntry& entry) {
	parse_item_entry(L, n, entry);
	entry.stackable = false;
//	entry.effect_modifiers; TODO
	entry.use_action = LuaAction(LuaValue());
	entry.stat_modifiers = parse_stat_modifiers(n);
	entry.cooldown_modifiers = parse_cooldown_modifiers(n);
	std::string type = parse_defaulted(n, "equipment_type", std::string());
	if (type == "armour") {
		entry.type = EquipmentEntry::BODY_ARMOUR;
	} else if (type == "ring") {
		entry.type = EquipmentEntry::RING;
	} else if (type == "boots") {
		entry.type = EquipmentEntry::BOOTS;
	} else if (type == "helmet") {
		entry.type = EquipmentEntry::HELMET;
	} else if (type == "gloves") {
		entry.type = EquipmentEntry::GLOVES;
	} else {
		LANARTS_ASSERT(type.empty());
	}
}

void load_equipment_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	EquipmentEntry* entry = new EquipmentEntry;
	parse_equipment_entry(L, node, *entry);

	game_item_data.push_back(entry);
	/* Lua loading code */
	lua_pushyaml(L, node);
	lua_pushvalue(L, -1); // Duplicate
	(*value)[entry->name].pop();
	lua_pushstring(L, "armour");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);

}

void load_equipment_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	load_data_impl_template(filenames, "equipment", load_equipment_callbackf, L,
			itemtable);
}
