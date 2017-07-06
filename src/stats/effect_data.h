#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include <lcommon/LuaLazyValue.h>
#include <ldraw/Colour.h>
#include <data/ResourceDataSet.h>

#include "lanarts_defines.h"

#include "AllowedActions.h"

#include "stats.h"

struct EffectEntry {
	std::string name, category;
	LuaValue stat_func, draw_func, attack_stat_func, init_func, step_func;
	// TODO Move rest to lua only ^
    // For stat listings when hovering over items:
    LuaValue console_draw_func;
    LuaValue remove_func, remove_derived_func; // When remove is called, should set active = false.
    // For most effects, remove_func == init_func is fine.
    LuaValue apply_derived_func, apply_buff_func;
    LuaValue on_melee_func, on_damage_func, on_receive_melee_func;
	// Equipment effect callbacks. Can be used to associate with equipment granting this.
	LuaValue on_equip_func, on_identify_func, on_uncurse_func; // TODO for latter two
	// Spell effect callbacks. Can be used to associate with spells granting this.
	LuaValue on_gain_spell_func;

	// The object we parse all our contents from:
	LuaValue raw_lua_object;

	Colour effected_colour;
	sprite_id effected_sprite = -1;
	AllowedActions allowed_actions;
	int fade_out = 0;
	bool additive_duration = false;
};

effect_id get_effect_by_name(const char* name);
extern ResourceDataSet<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
