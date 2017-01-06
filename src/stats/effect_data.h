#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include <lcommon/LuaLazyValue.h>
#include <ldraw/Colour.h>

#include "lanarts_defines.h"

#include "AllowedActions.h"

#include "stats.h"

struct EffectEntry {
	std::string name, category;
	LuaLazyValue stat_func, draw_func, attack_stat_func, init_func, finish_func, step_func;
	// TODO Move rest to lua only ^
        LuaValue on_melee_func, on_damage_func, on_receive_melee_func;
	// Equipment effect callbacks. Can be used to associate with equipment granting this.
	LuaValue on_equip_func, on_identify_func, on_uncurse_func; // TODO for latter two
	// Spell effect callbacks. Can be used to associate with spells granting this.
	LuaValue on_gain_spell_func;

	Colour effected_colour;
	sprite_id effected_sprite = -1;
	AllowedActions allowed_actions;
	int fade_out = 0;
	bool additive_duration = false;

	void init(lua_State* L) {
		init_func.initialize(L);
		finish_func.initialize(L);
        stat_func.initialize(L);
        draw_func.initialize(L);
		attack_stat_func.initialize(L);
		step_func.initialize(L);
	}
};

effect_id get_effect_by_name(const char* name);
extern std::vector<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
