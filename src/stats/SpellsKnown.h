/*
 * SpellsKnown.h:
 *  Represents spells that a combat entity can use
 */

#ifndef SPELLSKNOWN_H_
#define SPELLSKNOWN_H_

#include <vector>

#include "../lanarts_defines.h"

struct SpellEntry;
class SerializeBuffer;

class SpellsKnown {
public:
	spell_id get(int ind) {
		return spells.at(ind);
	}
	SpellEntry& get_entry(int ind);

	size_t amount() {
		return spells.size();
	}

	void add_spell(spell_id slot);
	void remove_spell(spell_id slot);
	bool has_spell(spell_id slot);

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

private:
	std::vector<spell_id> spells;
};

#endif /* SPELLSKNOWN_H_ */
