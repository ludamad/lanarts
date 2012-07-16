/*
 * SpellsKnown.h:
 *  Represents spells that a combat entity can use
 */

#ifndef SPELLSKNOWN_H_
#define SPELLSKNOWN_H_

#include <vector>

#include "../util/game_basic_structs.h"

class SpellsKnown {
public:
	spell_id get(int ind) { return spells.at(ind); }
	size_t amount() { return spells.size(); }

	void add_spell(spell_id slot);
	void remove_spell(spell_id slot);
	bool has_spell(spell_id slot);
private:
	std::vector<spell_id> spells;
};

#endif /* SPELLSKNOWN_H_ */
