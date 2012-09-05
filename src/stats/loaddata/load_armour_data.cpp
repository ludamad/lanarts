#include <fstream>

extern "C" {
#include <lua/lua.h>
}

#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../item_data.h"

#include "load_stats.h"

using namespace std;
void _load_armour_item_entries() {
	const int default_radius = 11;

	//Create items from projectiles
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<EquipmentEntry*>(&ientry)) {
			EquipmentEntry& entry = get_equipment_entry(i);
			//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
			_game_item_data.push_back(
					_ItemEntry(entry.name, entry.description, "",
							default_radius, entry.item_sprite, "equip", "",
							false, entry.shop_cost, _ItemEntry::ALWAYS_KNOWN,
							_ItemEntry::ARMOUR, i));
		}
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
