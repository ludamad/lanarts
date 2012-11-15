/*
 * SpellsKnown.cpp:
 *  Represents spells that a combat entity can use
 */

#include <algorithm>

#include <common/SerializeBuffer.h>
#include "SpellsKnown.h"

#include "SpellEntry.h"

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

SpellEntry& SpellsKnown::get_entry(int ind) {
	return game_spell_data.at(get(ind));
}

bool SpellsKnown::has_spell(spell_id slot) {
	std::vector<spell_id>::iterator it = std::find(spells.begin(), spells.end(),
			slot);
	return (it != spells.end());
}

void SpellsKnown::serialize(SerializeBuffer& serializer) {
	serializer.write_container(spells);
}

void SpellsKnown::deserialize(SerializeBuffer& serializer) {
	serializer.read_container(spells);
}

