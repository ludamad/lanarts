/*
 * ClassEntry.h:
 *  Represents a class progression
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include <lcommon/strformat.h>

#include "data/game_data.h"
#include "stats/stat_formulas.h"

#include "SpellEntry.h"
#include "ClassEntry.h"

sprite_id ClassEntry::get_sprite() {
	return sprites.at(0);
}

const char* ClassEntry::entry_type() {
	return "Class";
}

static ClassSpell parse_class_spell(const LuaField& value) {
	ClassSpell spell;

	spell.spell = res::spell_id(value["spell"].to_str());
	spell.xplevel_required = value["level_needed"].to_int();

	return spell;
}

static ClassSpellProgression parse_class_spell_progression(
		const LuaField& value) {
	ClassSpellProgression progression;

	int valuelen = value.objlen();
	for (int i = 1; i <= valuelen; i++) {
		progression.available_spells.push_back(parse_class_spell(value[i]));
	}

	return progression;
}

static void parse_gain_per_level(ClassEntry& entry,
		const LuaField& value) {
	using namespace luawrap;
	entry.hp_perlevel = defaulted(value, "hp", 0);
	entry.mp_perlevel = defaulted(value, "mp", 0);

	entry.str_perlevel = defaulted(value, "strength", 0);
	entry.def_perlevel = defaulted(value, "defence", 0);
	entry.mag_perlevel = defaulted(value, "magic", 0);
	entry.will_perlevel = defaulted(value, "willpower", 0);

	entry.mpregen_perlevel = defaulted(value, "mpregen", 0.0f);
	entry.hpregen_perlevel = defaulted(value, "hpregen", 0.0f);
}

void ClassEntry::parse_lua_table(const LuaValue& table) {
	parse_gain_per_level(*this, table["gain_per_level"]);

	spell_progression = parse_class_spell_progression(
			table["available_spells"]);
	starting_stats = parse_combat_stats(table["start_stats"]);

	LuaField sprites = table["sprites"];
	int sprite_len = sprites.objlen();
	for (int i = 1; i <= sprite_len; i++) {
		this->sprites.push_back(res::sprite_id(sprites[i].to_str()));
	}
}
namespace res {
	::class_id class_id(const char* name) {
		return get_class_by_name(name);
	}

	::class_id class_id(const std::string& name) {
		return get_class_by_name(name.c_str());
	}

	ClassEntry class_entry(const char* name) {
		return game_class_data.at(class_id(name));
	}

	ClassEntry class_entry(const std::string& name) {
		return game_class_data.at(class_id(name));
	}

	ClassEntry class_entry(::class_id id) {
		return game_class_data.at(id);
	}
}
