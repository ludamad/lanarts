#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include "../lua/LuaValue.h"
#include "../lanarts_defines.h"

#include "../stats/stats.h"

struct EffectEntry {
	std::string name;
	LuaValue stat_func;
	LuaValue init_func;
	LuaValue step_func;
	bool additive_duration;

	void init(lua_State* L) {
		init_func.initialize(L);
		stat_func.initialize(L);
		step_func.initialize(L);
	}
};

effect_id get_effect_by_name(const char* name);
extern std::vector<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
