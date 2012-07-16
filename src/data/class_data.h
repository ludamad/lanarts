/*
 * class_data.h:
 *  Contains data related to class progression
 */

#ifndef CLASS_DATA_H_
#define CLASS_DATA_H_

#include <string>
#include <vector>

#include "../gamestats/combat_stats.h"

#include "../util/game_basic_structs.h"

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

struct ClassType {
	std::string name;
	ClassSpellProgression spell_progression;
	CombatStats starting_stats;
	int hp_perlevel, mp_perlevel;
	int str_perlevel, def_perlevel, mag_perlevel, will_perlevel;
	float mpregen_perlevel, hpregen_perlevel;
	ClassType() :
			hp_perlevel(0), mp_perlevel(0), str_perlevel(0), def_perlevel(0), mag_perlevel(
					0), will_perlevel(0), mpregen_perlevel(0), hpregen_perlevel(
					0) {
	}
};

extern std::vector<ClassType> game_class_data;
class_id get_class_by_name(const char* name);

#endif /* CLASS_DATA_H_ */
