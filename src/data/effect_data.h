/*
 * effect_data.h
 *
 *  Created on: March 24, 2012
 *      Author: 100397561
 */
#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include "../gamestats/Stats.h"

enum {
	EFFECT_HASTE = 0
};

typedef void (*effect_actionf)(Stats& basestats, Stats& affected);

struct Effect {
	const char* name;
	int length;
	effect_actionf action;
	Effect(const char* name, int length, effect_actionf act) :
		name(name), length(length), action(act){
	}
};

extern Effect game_effect_data[];
extern size_t game_effect_n;

#endif /* EFFECT_DATA_H_ */