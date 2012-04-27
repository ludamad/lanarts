/*
 * spell_data.h
 *  Defines the data that goes into a spell
 */

#ifndef SPELL_DATA_H_
#define SPELL_DATA_H_

#include <string>

struct SpellData;

typedef bool (*spell_effectf)(GameState* state, const SpellData& spell, GameInst* caster);


struct SpellData {
	std::string name;
	int sprite;
	spell_effectf effect;
};

#endif /* SPELL_DATA_H_ */
