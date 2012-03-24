#ifndef EFFECTS_H
#define EFFECTS_H

#include "Stats.h"
#include "../data/effect_data.h"

#define EFFECTS_NUMBER 40

struct effect {
	int effect;
	int t_remaining;
};

struct Effects {
	Effects() {
		for (int i = 0; i < EFFECTS_NUMBER; i++) {
			effects[i].t_remaining = 0;
		}
	}
	void add(int effect, int length) {
		for (int i = 0; i < EFFECTS_NUMBER; i++) {
			if (effects[i].t_remaining == 0 || effects[i].effect == effect) {
				effect[i].effect = effect;
				effect[i].t_remaining += length;
				return;
			}
		}
	}
	
	void process(Stats& basestats, Stats& affected ){
		for(int i = 0; i < game_effect_n; i++){
			if(effects[i].t_remaining > 0)
				game_effect_data[effects[i].effect].action(basestats, affected);
		}
	}
	
	effect effects[EFFECTS_NUMBER];
};

#endif // EFFECTS_H