/*
 * effect_data.h
 *
 *  Created on: March 24, 2012
 *      Author: 100397561
 */
#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <vector>

#include "../gamestats/Stats.h"

enum {
	EFFECT_HASTE = 0
};

typedef void (*effect_actionf)(Stats& basestats, Stats& affected);

struct EffectType {
	const char* name;
	effect_actionf action;
	EffectType(const char* name, effect_actionf act) :
		name(name), action(act){
	}
};

extern std::vector<EffectType> game_effect_data;

#endif /* EFFECT_DATA_H_ */
