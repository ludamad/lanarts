/*
 * SpellsKnown.cpp:
 *  Represents spells that a combat entity can use
 */

#include <algorithm>

#include "SpellsKnown.h"

void SpellsKnown::add_spell(spell_id slot) {
	spells.push_back(slot);
}

void SpellsKnown::remove_spell(spell_id slot) {
	std::vector<spell_id>::iterator it = std::find(spells.begin(), spells.end(),
			slot);
	if (it != spells.end()) {
		spells.erase(it);
	}
}

bool SpellsKnown::has_spell(spell_id slot) {
	std::vector<spell_id>::iterator it = std::find(spells.begin(), spells.end(),
			slot);
	return (it != spells.end());
}

