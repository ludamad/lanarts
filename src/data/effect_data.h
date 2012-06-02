/*
 * effect_data.h
 *
 *  Created on: March 24, 2012dw
 *      Author: 100397561
 */
#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include "../util/LuaValue.h"
#include "../gamestats/Stats.h"

struct EffectEntry {
	const char* name;
	LuaValue statmod;
	EffectEntry(const char* name, const std::string& statmod_luaf) :
		name(name), statmod(statmod_luaf){
	}
	void init(lua_State* L){
		statmod.initialize(L);
	}
};

int get_effect_by_name(const char* name);
extern std::vector<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
