/*
 * spell_data.h
 *  Defines the data that goes into a spell
 */

#ifndef SPELL_DATA_H_
#define SPELL_DATA_H_

#include <string>

#include "../util/LuaValue.h"

//This never got to be used:
//typedef bool (*spell_effectf)(GameState* state, const SpellData& spell, GameInst* caster);

struct SpellEntry {
	std::string name;
	int sprite;
	LuaValue effect;//Immediate effect
	LuaValue projectile_init;
	LuaValue projectile_step;
	SpellEntry(const std::string& name, int sprite, const std::string& effect);
	void init(lua_State* L){
		effect.initialize(L);
	}
};

extern std::vector<SpellEntry> game_spell_data;

#endif /* SPELL_DATA_H_ */
