#include <fstream>
#include "yaml_util.h"
#include "../game_data.h"
#include "../item_data.h"

using namespace std;

void load_armour_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ArmourEntry entry;
	entry.damage_reduction = parse_defaulted(node, "reduction", CoreStatMultiplier());
	entry.resistance = parse_defaulted(node, "resistance", CoreStatMultiplier());
	entry.magic_reduction = parse_defaulted(node, "magic_reduction", CoreStatMultiplier());
	entry.magic_resistance = parse_defaulted(node, "magic_resistance", CoreStatMultiplier());
	entry.spell_cooldown_multiplier = parse_defaulted(node, "spell_cooldown_multiplier", 1.0f);
	entry.item_sprite = parse_sprite_number(node, "spr_item");

	game_armour_data.push_back(entry);
	if (value)
		value->table_set_yaml(L, game_armour_data.back().name, &node);
}

void load_armour_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	game_armour_data.clear();
	load_data_impl_template(filenames, "armours", load_armour_callbackf, L,
			itemtable);
}

void load_armour_item_entries() {
	const int default_radius = 11;

	//Create items from armours
	for (int i = 0; i < game_armour_data.size(); i++) {
		ArmourEntry& entry = game_armour_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(
				ItemEntry(entry.name, "", default_radius, entry.item_sprite,
						"equip", "", false, entry.equiptype, i));
	}
}
