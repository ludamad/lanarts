/*
 * ClassEntry.h:
 *  Represents a class progression
 */

#ifndef CLASSENTRY_H_
#define CLASSENTRY_H_

#include <string>
#include <vector>

#include <luawrap/LuaValue.h>

#include "data/ResourceEntryBase.h"

#include "lanarts_defines.h"

#include "combat_stats.h"

struct ClassSpell {
	int xplevel_required;
	spell_id spell;
};

struct ClassSpellProgression {
	std::vector<ClassSpell> available_spells;
	std::vector<spell_id> spells_available_at_level(int level) const {
		std::vector<spell_id> ret;
		for (int i = 0; i < available_spells.size(); i++) {
			if (available_spells[i].xplevel_required <= level) {
				ret.push_back(available_spells[i].spell);
			}
		}
		return ret;
	}
};

struct ClassEntry: public ResourceEntryBase {
	int class_id; // Set during init!

	ClassSpellProgression spell_progression;
	CombatStats starting_stats;
	std::vector<sprite_id> sprites;

	int hp_perlevel, mp_perlevel;
	int str_perlevel, def_perlevel, mag_perlevel, will_perlevel;
	float mpregen_perlevel, hpregen_perlevel;

	ClassEntry() :
					class_id(-1),
					hp_perlevel(0),
					mp_perlevel(0),
					str_perlevel(0),
					def_perlevel(0),
					mag_perlevel(0),
					will_perlevel(0),
					mpregen_perlevel(0),
					hpregen_perlevel(0) {
	}

	virtual sprite_id get_sprite();
	virtual const char* entry_type();

	virtual void parse_lua_table(const LuaValue& table);
};

extern std::vector<ClassEntry> game_class_data;
class_id get_class_by_name(const char* name);

namespace res {
	::class_id class_id(const char* name);
	::class_id class_id(const std::string& name);
	ClassEntry class_entry(const char* name);
	ClassEntry class_entry(const std::string& name);
	ClassEntry class_entry(::class_id id);
}

#endif /* CLASSENTRY_H_ */
