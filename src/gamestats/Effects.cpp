#include "Effects.h"
#include "../lua/lua_api.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}


void Effects::process(lua_State* L, Stats & basestats, Stats & affected) {
	lua_pushstats(L, basestats);
	lua_pushstats(L, affected);

	int affind = lua_gettop(L);
	int baseind = affind-1;

	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			game_effect_data[effects[i].effect].statmod.push(L);
			lua_pushvalue(L, affind);
			lua_pushvalue(L, baseind);
			lua_call(L, 2, 0);
		}
	}
	basestats = *lua_getstats(L, baseind);
	affected = *lua_getstats(L, affind);
	lua_pop(L, 2);//pop base&affected
}

void Effects::step() {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0)
			effects[i].t_remaining--;

	}
}

Effect *Effects::get(int effect) {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0 && effects[i].effect == effect) {
			return &effects[i];
		}
	}

	return NULL;
}

void Effects::add(int effect, int length) {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining == 0 || effects[i].effect == effect) {
			effects[i].effect = effect;
			effects[i].t_remaining += length;
			return;
		}
	}

}

