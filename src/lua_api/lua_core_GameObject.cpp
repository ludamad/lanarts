/*
 * lua_core_GameObject.cpp:
 *  Representation of a GameInst in Lua.
 */

#include <lua.hpp>
#include <stdexcept>
#include <typeinfo>

#include <lua_api/lua_api.h>

#include <luawrap/luawrap.h>
#include <luawrap/members.h>

#include <luawrap/macros.h>

#include "data/lua_game_data.h"

#include "gamestate/GameState.h"

#include "data/game_data.h"

#include "objects/GameInst.h"
#include "objects/AnimatedInst.h"
#include "objects/EnemyInst.h"
#include "objects/FeatureInst.h"
#include "objects/StoreInst.h"
#include "objects/ItemInst.h"
#include "objects/PlayerInst.h"
#include "objects/CombatGameInst.h"
#include "objects/ProjectileInst.h"

#include "util/map_container.h"

#include "lua_api.h"

#include "draw/SpriteEntry.h"

static int lapi_gameinst_stats(lua_State* L) {
	lua_push_combatstats(L, luawrap::get<GameInst*>(L, 1));
	return 1;
}

static int lapi_gameinst_effectivestats(lua_State* L) {
    lua_push_effectivestats(L, luawrap::get<GameInst*>(L, 1));
    return 1;
}

static int object_init(lua_State* L);
static void player_init(LuaStackValue obj, Pos xy, std::string name);

/* Use a per-instance lua table as a fallback */
static int lapi_gameinst_getter_fallback(lua_State* L) {
//	LuaValue& variables = luawrap::get<GameInst*>(L, 1)->lua_variables;
//	if (variables.empty()) {
//		lua_pushnil(L);
//	} else {
//		variables.push();
//		lua_pushvalue(L, 2); // On stack: [object table, key]
//		lua_rawget(L, -2);
//	}
	/* Try to access in 'self.type'. */
	/* Get type table */
	lua_pushliteral(L, "type");
	lua_rawget(L, 1);

	LuaStackValue table(L, -1);
	if (!lua_isnil(L, -1)) {
		// First, try the __index method.
		lua_getfield(L, table.index(), "__index");
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 2);
			lua_call(L, 2, 1); // On stack [type table, value]
		}
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1); // pop nil
			lua_pushvalue(L, 2); // On stack: [type table, key]
			lua_gettable(L, table.index()); // On stack [type table, value]
		}
	}
	return 1;
}

/* Use a per-instance lua table as a fallback */
static int lapi_gameinst_setter_fallback(lua_State* L) {
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	lua_rawset(L, 1);
	return 1;
}

static int lapi_gameinst_freeref(lua_State* L) {
	auto user_data = (GameInst**) lua_touserdata(L, 1);
	GameInst::free_reference(*user_data);
	return 0;
}

static int lapi_gameinst_map(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->game_world().push_level_object(luawrap::get<GameInst*>(L, 1)->current_floor);
	return 1;
}

static void lapi_gameinst_remove_effect(GameInst* inst, LuaStackValue name) {
    Effect* eff = inst->effects.get_active(name.to_str());
    // Trigger finish functions and set t_remaining to 0.
    // (Does nothing if eff is NULL.)
    if (eff) {
        inst->effects.remove(lua_api::gamestate(name), inst, eff);
    }
}

static LuaValue lua_gameinst_base_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(GameInst*);

	LuaValue meta = luameta_new(L, "GameObject");
	meta["__isgameinst"] = true;

	LuaValue methods = luameta_constants(meta);
	meta["init"].bind_function(object_init);
	methods["init"] = meta["init"];

	LUAWRAP_METHOD(methods, step, OBJ->step(lua_api::gamestate(L)));
	LUAWRAP_METHOD(methods, draw, OBJ->draw(lua_api::gamestate(L)));

	LuaValue getters = luameta_getters(meta);
	LUAWRAP_GETTER(getters, xy, OBJ->pos());
	LUAWRAP_GETTER(getters, xy_previous, Pos(OBJ->last_x, OBJ->last_y));
	LUAWRAP_GETTER(getters, tile_xy, Pos(OBJ->x / TILE_SIZE, OBJ->y / TILE_SIZE));
	luawrap::bind_getter(getters["x"], &GameInst::x);
	luawrap::bind_getter(getters["y"], &GameInst::y);
	luawrap::bind_getter(getters["__table"], &GameInst::lua_variables);
	luawrap::bind_getter(getters["x_previous"], &GameInst::last_x);
	luawrap::bind_getter(getters["y_previous"], &GameInst::last_y);
	luawrap::bind_getter(getters["destroyed"], &GameInst::destroyed);
	luawrap::bind_getter(getters["id"], &GameInst::id);
	luawrap::bind_getter(getters["depth"], &GameInst::depth);
	luawrap::bind_getter(getters["current_floor"], &GameInst::current_floor);
	luawrap::bind_getter(getters["radius"], &GameInst::radius);
    luawrap::bind_getter(getters["solid"], &GameInst::solid);
	luawrap::bind_getter(getters["target_radius"], &GameInst::target_radius);
	getters["map"].bind_function(lapi_gameinst_map);
	LUAWRAP_GETTER(meta, __tostring, typeid(*OBJ).name());

	// Use table as fallback for getting values
	lua_pushcfunction(L, &lapi_gameinst_getter_fallback);
	luameta_defaultgetter(meta, LuaStackValue(L, -1));
	lua_pop(L, 1);

	LuaValue setters = luameta_setters(meta);
	luawrap::bind_setter(setters["radius"], &GameInst::radius);
	luawrap::bind_setter(setters["target_radius"], &GameInst::target_radius);
	LUAWRAP_SETTER(setters, x, double, OBJ->update_position(VAL, OBJ->y));
	LUAWRAP_SETTER(setters, y, double, OBJ->update_position(OBJ->x, VAL));
	LUAWRAP_SETTER(setters, xy, PosF, OBJ->update_position(VAL.x, VAL.y));

    methods["remove_effect"].bind_function( lapi_gameinst_remove_effect);

    LUAWRAP_GETTER(methods, add_effect, OBJ->effects.add(lua_api::gamestate(L), OBJ, StatusEffect {effect_from_lua(L, 2), LuaValue(L, 3)}) );
    LUAWRAP_GETTER(methods, has_effect, OBJ->effects.has(lua_tostring(L, 2)));
    LUAWRAP_GETTER(methods, get_effect, OBJ->effects.get(lua_api::gamestate(L), OBJ, lua_tostring(L, 2)).state);
    LUAWRAP_GETTER(methods, has_effect_category, OBJ->effects.has_category(luawrap::get<const char*>(L, 2)));

	// Use table as fallback for setting values
	lua_pushcfunction(L, &lapi_gameinst_setter_fallback);
	luameta_defaultsetter(meta, LuaStackValue(L, -1));
	lua_pop(L, 1);

	return meta;
}

static int lapi_combatgameinst_damage(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	EffectiveAttackStats attack;
	int nargs = lua_gettop(L);
	attack.damage = lua_tonumber(L, 2);
	attack.power = lua_tonumber(L, 3);
	attack.magic_percentage = nargs >= 4 ? lua_tonumber(L, 4) : 1.0f;
	CombatGameInst* attacker = nargs >= 5 ? luawrap::get<CombatGameInst*>(L, 5) : NULL;
	attack.type_multiplier = nargs >= 6 ? lua_tonumber(L, 6) : 1.0f;

	bool died = (luawrap::get<CombatGameInst*>(L, 1)->damage(lua_api::gamestate(L), attack, attacker));
	lua_pushboolean(L, died);
	return 1;
}

static void lapi_combatgameinst_heal_hp(CombatGameInst* inst, float hp) {
	CoreStats& core = inst->core_stats();
	CoreStats& ecore = inst->effective_stats().core;
	core.heal_hp(hp, ecore.max_hp);
	ecore.hp = core.hp;
}

static void lapi_combatgameinst_heal_mp(CombatGameInst* inst, float mp) {
	CoreStats& core = inst->core_stats();
	CoreStats& ecore = inst->effective_stats().core;
	core.heal_mp(mp, ecore.max_mp);
	ecore.mp = core.mp;
}

static int lapi_do_nothing(lua_State *L) {
    return 0;
}

static bool go_towards_if_free(GameState* gs, CombatGameInst* inst, float vx, float vy) {
    float tx = vx, ty = vy;
    float nx = inst->x + tx, ny = inst->y + ty;
    if (!gs->solid_test(inst, iround(nx), iround(ny), inst->radius)) {
        inst->update_position(nx, ny);
        return true;
    }
    return false;
}

static bool lapi_go_towards_if_free(LuaStackValue obj, PosF dir) {
    return go_towards_if_free(lua_api::gamestate(obj), obj.as<CombatGameInst*>(), dir.x, dir.y);
}

static LuaValue lua_combatgameinst_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(CombatGameInst*);

	LuaValue meta = lua_gameinst_base_metatable(L);

    LuaValue getters = luameta_getters(meta);
    LuaValue setters = luameta_setters(meta);
	luawrap::bind_getter(getters["vx"], &CombatGameInst::vx);
        luawrap::bind_getter(getters["vy"], &CombatGameInst::vy);
	luawrap::bind_getter(getters["is_resting"], &CombatGameInst::is_resting);
    LUAWRAP_GETTER(getters, sprite, game_sprite_data.get(OBJ->get_sprite()).sprite);
	LUAWRAP_GETTER(getters, sprite_name, game_sprite_data.get(OBJ->get_sprite()).name);
    luawrap::bind_getter(getters["team"], &CombatGameInst::team);
    luawrap::bind_getter(getters["vision_radius"], &CombatGameInst::vision_radius);
    luawrap::bind_setter(setters["vision_radius"], &CombatGameInst::vision_radius);
	getters["stats"].bind_function(lapi_gameinst_stats);
	LUAWRAP_SETTER(setters, sprite_name, std::string, OBJ->get_sprite() = res::sprite_id(VAL));

	LuaValue methods = luameta_constants(meta);
    methods["is_combat_object"] = true;
	methods["learn_spell"] = [=](CombatGameInst* inst, const char* spellname) {
		auto& spell = game_spell_data.get(spellname);
		if (inst->stats().spells.has_spell(spell.id)) {
			return;
		}
		inst->stats().spells.add_spell(spell.id);
	};

	methods["inventory_get"] = [=](CombatGameInst* inst, int slot_idx) {
		LuaValue ret(L);
		if (slot_idx < 0 || slot_idx >= inst->inventory().max_size()) {
			return ret;
		}
		ret.newtable();
		ItemSlot& slot = inst->inventory().get(slot_idx);
		if (!slot.empty()) {
			ret["type"] = slot.item_entry().name;
		}
		ret["amount"] = slot.item.amount;
		ret["equipped"] = slot.is_equipped();
		return ret;
	};

	methods["inventory_set"] = [=](CombatGameInst* inst, int slot_idx, LuaStackValue slot_data) {
		if (slot_idx < 0 || slot_idx >= inst->inventory().max_size()) {
			return false;
		}
		ItemSlot& slot = inst->inventory().get(slot_idx);
		bool equipped = slot_data["equipped"].to_bool();
		// Before modifications, try unequip
		if (!equipped && slot.is_equipped()) {
			inst->inventory().deequip(slot_idx);
		}
		if (slot_data.has("type")) {
			slot.item = Item {
				get_item_by_name(slot_data["type"].to_str()),
				slot_data["amount"].to_int()
			};
		} else {
			slot.clear();
		}
		// After modifications, try equip
		if (equipped && !slot.is_equipped() && slot.item.is_equipment()) {
			inst->inventory().equip(slot_idx, true);
		}
		return true;
	};

	methods["effective_stats"].bind_function(lapi_gameinst_effectivestats);
    LUAWRAP_GETTER(getters, weapon_range, OBJ->equipment().weapon().weapon_entry().range());
    methods["go_towards_if_free"].bind_function(lapi_go_towards_if_free);

    LUAWRAP_METHOD(methods, heal_fully, OBJ->stats().core.heal_fully());
    LUAWRAP_METHOD(methods, gain_xp_from, OBJ->gain_xp_from(lua_api::gamestate(L), luawrap::get<CombatGameInst*>(L, 2)));
	LUAWRAP_METHOD(methods, team_gain_xp, OBJ->team_gain_xp(lua_api::gamestate(L), lua_tonumber(L, 2)));
	LUAWRAP_METHOD(methods, die, OBJ->die(lua_api::gamestate(L)));
	LUAWRAP_GETTER(getters, can_rest, OBJ->cooldowns().can_rest());
	LUAWRAP_METHOD(methods, direct_damage, OBJ->damage(lua_api::gamestate(L), lua_tonumber(L, 2), lua_gettop(L) < 3 ? NULL : luawrap::get<CombatGameInst*>(L, 3)));
	LUAWRAP_METHOD(methods, melee, luawrap::push(L, OBJ->melee_attack(lua_api::gamestate(L), luawrap::get<CombatGameInst*>(L, 2), OBJ->equipment().weapon(), true,
            // Damage multiplier:
            luawrap::get_defaulted(L, 3, 1.0f))) );
	LUAWRAP_GETTER(methods, has_melee_weapon, !OBJ->equipment().weapon().weapon_entry().uses_projectile);
	LUAWRAP_GETTER(methods, has_ranged_weapon, OBJ->equipment().weapon().weapon_entry().uses_projectile);
    LUAWRAP_METHOD(methods, projectile_attack, OBJ->projectile_attack(lua_api::gamestate(L), NULL, Weapon(), Item(get_projectile_by_name(lua_tostring(L, 2)))));
	LUAWRAP_METHOD(methods, use_spell, OBJ->use_spell(lua_api::gamestate(L), game_spell_data.get(lua_tostring(L, 2)), luawrap::get<Pos>(L, 3), luawrap::get_defaulted<GameInst*>(L, 4, nullptr)));
	LUAWRAP_METHOD(methods, try_use_spell, OBJ->try_use_spell(lua_api::gamestate(L), game_spell_data.get(lua_tostring(L, 2)), luawrap::get<Pos>(L, 3), luawrap::get_defaulted<GameInst*>(L, 4, nullptr)));
	LUAWRAP_METHOD(methods, use_mp, OBJ->use_mp(lua_api::gamestate(L), lua_tonumber(L, 2)));

    methods["damage"].bind_function(lapi_combatgameinst_damage);
	methods["heal_hp"].bind_function(lapi_combatgameinst_heal_hp);
    methods["heal_mp"].bind_function(lapi_combatgameinst_heal_mp);

    LUAWRAP_GETTER(methods, is_local_player, false);

	methods["reset_rest_cooldown"].bind_function(lapi_do_nothing);

        methods["inventory"] = [=](CombatGameInst* inst) -> LuaValue {
            LuaValue table = LuaValue::newtable(L);
            auto& inv = inst->inventory();
            for (int i = 0; i < inv.max_size(); i++) {
                if (!inv.slot_filled(i)) {
                    continue;
                }
                LuaValue entry = LuaValue::newtable(L);
                entry["item"] = inv.get(i).item_entry().name;
                entry["amount"] = inv.get(i).amount();
                entry["slot"] = i;
                table[table.objlen() + 1] = entry;
            }
            return table;
        };

	return meta;
}

static LuaValue lua_feature_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(FeatureInst*);

	LuaValue meta = lua_gameinst_base_metatable(L);

        LuaValue getters = luameta_getters(meta);
	LuaValue methods = luameta_constants(meta);
        LUAWRAP_GETTER(getters, has_been_used, OBJ->has_been_used());
	LUAWRAP_METHOD(methods, interact, OBJ->player_interact(lua_api::gamestate(L), luawrap::get<GameInst*>(L, 2)));
	return meta;
}


static LuaValue lua_iteminst_metatable(lua_State* L) {
    LUAWRAP_SET_TYPE(ItemInst*);

    LuaValue meta = lua_gameinst_base_metatable(L);
    LuaValue getters = luameta_getters(meta);
    LUAWRAP_GETTER(getters, type, OBJ->item_type().item_entry().name);
    LUAWRAP_GETTER(getters, amount, OBJ->item_quantity());
    return meta;
}

static LuaValue lua_projectileinst_metatable(lua_State* L) {
    LUAWRAP_SET_TYPE(ProjectileInst*);

    LuaValue meta = lua_gameinst_base_metatable(L);
    LuaValue getters = luameta_getters(meta);
    getters["origin"] = [=](ProjectileInst* obj) {
    	return obj->origin(lua_api::gamestate(L));
    };
	LUAWRAP_GETTER(getters, vx, OBJ->vx);
	LUAWRAP_GETTER(getters, vy, OBJ->vy);
	LUAWRAP_GETTER(getters, speed, OBJ->speed);
    return meta;
}

static int lua_enemyinst_metatable_clone(lua_State* L);
static LuaValue lua_enemyinst_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(EnemyInst*);

	LuaValue meta = lua_combatgameinst_metatable(L);
	LuaValue methods = luameta_constants(meta), getters = luameta_getters(meta), setters = luameta_setters(meta);

	LUAWRAP_GETTER(getters, name, OBJ->etype().name);
    LUAWRAP_GETTER(getters, unique, OBJ->etype().unique);
	LUAWRAP_GETTER(getters, def, OBJ->etype().raw_table);
    LUAWRAP_GETTER(getters, is_enemy, true);
	LUAWRAP_GETTER(getters, kills, 0);
    LUAWRAP_GETTER(getters, xpworth, OBJ->xpworth());
    LUAWRAP_SETTER(setters, xpworth, double, OBJ->xpworth() = VAL);
    meta["clone"].bind_function(lua_enemyinst_metatable_clone);
	return meta;
}
static int lua_enemyinst_metatable_clone(lua_State* L) {
    lua_enemyinst_metatable(L).push();
    return 1;
}

static void apply_melee_cooldown(PlayerInst* player) {
    auto estats = player->effective_stats();
    //if (player->equipment().has_projectile()) {
    //    cooldown_mult = estats.cooldown_modifiers.ranged_cooldown_multiplier;
    //} else {
    float cooldown_mult = estats.cooldown_modifiers.melee_cooldown_multiplier;
    int cooldown = player->weapon().weapon_entry().cooldown();
    //}
    player->cooldowns().reset_action_cooldown(cooldown * cooldown_mult);
}

static int direction_towards_unexplored(lua_State* L) {
    auto* p = LuaStackValue(L, 1).as<PlayerInst*>();
    bool finished = false;
    Pos dir = p->direction_towards_unexplored(lua_api::gamestate(L), &finished);
    if (finished) {
        lua_pushnil(L);
    } else {
        luawrap::push(L, dir);
    }
    return 1;
}

static LuaValue* ACTIVE_FILTER = NULL;

static bool _lua_collision_filterf(GameInst* inst1, GameInst* inst2) {
    lua_State * L = ACTIVE_FILTER->luastate();
    ACTIVE_FILTER->push();
    return luawrap::call<bool>(L, inst1, inst2);
}

static col_filterf get_lua_collision_filter(LuaValue& filter) {
    ACTIVE_FILTER = &filter;
    return &_lua_collision_filterf;
}

static Pos direction_towards_object(LuaStackValue player, LuaValue filter) {
    auto* p = player.as<PlayerInst*>();
    return p->direction_towards_object(lua_api::gamestate(player), get_lua_collision_filter(filter));
}

Pos configure_dir(GameState* gs, PlayerInst* inst, float dx, float dy);

static LuaValue lua_playerinst_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(PlayerInst*);
	LuaValue meta = lua_combatgameinst_metatable(L);
	LuaValue methods = luameta_constants(meta);
	meta["init"].bind_function(player_init);
	methods["init"] = meta["init"];

	LuaValue getters = luameta_getters(meta);
    LUAWRAP_GETTER(getters, is_enemy, false);
	LUAWRAP_GETTER(getters, name, OBJ->player_entry(lua_api::gamestate(L)).player_name);
	LUAWRAP_GETTER(getters, class_name, OBJ->class_stats().class_entry().name);
    LUAWRAP_GETTER(getters, kills, OBJ->score_stats().kills);
    LUAWRAP_GETTER(getters, target, lua_api::gamestate(L)->get_instance(OBJ->target()));
	LUAWRAP_GETTER(getters, deepest_floor, OBJ->score_stats().deepest_floor);
	LUAWRAP_GETTER(getters, deaths, OBJ->score_stats().deaths);
	LUAWRAP_GETTER(getters, def, OBJ->class_stats().class_entry().raw_table);
	LUAWRAP_GETTER(getters, spells, OBJ->stats().spells.spell_id_list());
	LUAWRAP_GETTER(getters, last_moved_direction, OBJ->last_moved_direction());
	LUAWRAP_GETTER(getters, is_ghost, OBJ->is_ghost());

	LuaValue setters = luameta_setters(meta);
	LUAWRAP_SETTER(setters, is_ghost, bool, OBJ->is_ghost() = VAL);
	LUAWRAP_SETTER(setters, input_source, LuaValue, OBJ->input_source().value = VAL);
	LUAWRAP_GETTER(getters, input_source, OBJ->input_source().value);
	LUAWRAP_GETTER(getters, weapon_sprite, res::sprite(OBJ->weapon().weapon_entry().item_sprite));

    methods["gain_gold"] = [=](PlayerInst* obj, int gold) {
        obj->gold(lua_api::gamestate(L)) += gold;
    };
	LUAWRAP_GETTER(methods, within_field_of_view, OBJ->within_field_of_view(luawrap::get<Pos>(L, 2)));
	LUAWRAP_GETTER(methods, is_local_player, OBJ->is_focus_player(lua_api::gamestate(L)));
	LUAWRAP_GETTER(methods, can_benefit_from_rest, OBJ->can_benefit_from_rest());
	methods["gain_level"] = [=](PlayerInst* obj) {
		obj->gain_xp(lua_api::gamestate(L), obj->class_stats().xpneeded - obj->class_stats().xp);
	};
    LUAWRAP_METHOD(methods, gain_xp, OBJ->stats().gain_xp(luawrap::get<int>(L,2), OBJ));;//players_gain_xp(lua_api::gamestate(L), luawrap::get<int>(L, 2)));
	LUAWRAP_METHOD(methods, reset_rest_cooldown, OBJ->cooldowns().reset_rest_cooldown(REST_COOLDOWN));
	methods["inventory_sell"] = [=](PlayerInst* inst, int slot_idx) {
		GameAction action;
		action.use_id = slot_idx;
		inst->sell_item(lua_api::gamestate(L), action);
	};

    methods["apply_melee_cooldown"].bind_function(apply_melee_cooldown);
    methods["direction_towards_unexplored"].bind_function(direction_towards_unexplored);
    methods["direction_towards_object"].bind_function(direction_towards_object);
	methods["configure_dir"] = [=](PlayerInst* inst, PosF xy) -> Pos {
		return configure_dir(lua_api::gamestate(L), inst, xy.x, xy.y);
	};

	return meta;
}

static void lua_gameinst_push_metatable(lua_State* L, GameInst* inst) {
	if (dynamic_cast<PlayerInst*>(inst)) {
		luameta_push(L, &lua_playerinst_metatable);
	} else if (dynamic_cast<EnemyInst*>(inst)) {
		luameta_push(L, &lua_enemyinst_metatable);
	} else if (dynamic_cast<CombatGameInst*>(inst)) {
		luameta_push(L, &lua_combatgameinst_metatable);
    } else if (dynamic_cast<ItemInst*>(inst)) {
        luameta_push(L, &lua_iteminst_metatable);
    } else if (dynamic_cast<ProjectileInst*>(inst)) {
        luameta_push(L, &lua_projectileinst_metatable);
	} else if (dynamic_cast<FeatureInst*>(inst)) {
		luameta_push(L, &lua_feature_metatable);
	} else {
		luameta_push(L, &lua_gameinst_base_metatable);
	}
}

static void create_and_push_gameinst_cache(lua_State* L) {
	// Create the cache table
	lua_newtable(L);
	int cache_idx = lua_gettop(L);

	lua_pushvalue(L, LUA_REGISTRYINDEX);
	int reg_idx = lua_gettop(L);
	lua_pushlightuserdata(L, (void*)&create_and_push_gameinst_cache);
	lua_pushvalue(L, cache_idx);
	lua_rawset(L, reg_idx);

	// Set up as weak table
	lua_newtable(L);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, cache_idx);

	lua_pop(L, 1); // Pop registry
}

// Functions required for luawrap
namespace GameInstWrap {
	GameInst* get(lua_State* L, int idx) {
		// Allow for any object to be passed, if it defines __objectref
		lua_getfield(L, idx, "__objectref");
		GameInst** udata = (GameInst**) lua_touserdata(L, -1);
		lua_pop(L, 1);
		return *udata;
	}

	bool check(lua_State* L, int idx) {
		lua_getmetatable(L, idx);
		if (lua_isnil(L, -1)) {
			return false;
		}
		lua_getfield(L, -1, "__isgameinst");
		bool isgameinst = !lua_isnil(L, -1);
		lua_pop(L, 2);
		return isgameinst;
	}

	void push_ref(lua_State* L, GameInst* inst);

	static int __save(lua_State* L) {
		GameState* gs = lua_api::gamestate(L);
		GameInst** udata = (GameInst**) lua_touserdata(L, 1);
//        if ((*udata)->destroyed) {
//            gs->post_deserialize_data().postpone_destroyed_instance_serialization(*udata);
//            lua_pushinteger(L, 0);
//            lua_pushinteger(L, (*udata)->current_floor);
//            LuaValue table = LuaValue::newtable(L);
//            if (!(*udata)->lua_variables.empty()) {
//                table[1] = (*udata)->lua_variables;
//            }
//            if (dynamic_cast<CombatGameInst*>(*udata)) {
//                for (auto& eff : dynamic_cast<CombatGameInst*>(*udata)->effects.effects) {
//                    if (!eff.state.empty()) {
//                        table[table.objlen() + 1] = eff.state;
//                    }
//                }
//            }
//            table.push();
//            return 2;
//        }

		GameInst* test_inst = NULL;
        if ((*udata)->id > 0) {
            test_inst = gs->get_level((*udata)->current_floor)->game_inst_set().get_instance((*udata)->id);
        } else {
            GameInstRef ref = gs->game_world().get_removed_object(-(*udata)->id);
            test_inst = ref.get();
        }
                if ((*udata) != test_inst) {
                    throw std::runtime_error(format("Attempt to save GameInst that cannot be found at level %d, id %d!", (*udata)->current_floor, (*udata)->id));
                }
		lua_pushinteger(L, (*udata)->id);
		lua_pushinteger(L, (*udata)->current_floor);
		return 2;
	}

    static LuaValue ref_metatable(lua_State* L);

	static int __load(lua_State* L) {
        GameState* gs = lua_api::gamestate(L);
	    // Get our coordinates:
        int id = luaL_checkinteger(L, 2);
        int current_floor = luaL_checkinteger(L, 3);
        // Create a dummy reference:
        GameInst** lua_inst = (GameInst**) lua_newuserdata(L, sizeof(GameInst*));
        *lua_inst = NULL;
        luameta_push(L, &ref_metatable);
        lua_setmetatable(L, -2);
        // Postpone deserialization until the entire game structure is created:
        gs->post_deserialize_data().postpone_instance_deserialization(lua_inst, current_floor, id);
            // Return a dummy reference, later filled in
            return 1;
	}

	static LuaValue ref_metatable(lua_State* L) {
		LuaValue meta = luameta_new(L, "GameInstRef");
		meta["__save"].bind_function(__save);
		meta["__load"].bind_function(__load);
		luameta_gc(meta, lapi_gameinst_freeref);
		return meta;
	}

	void push_ref(lua_State* L, GameInst* inst) {
	    // TODO proper lua lifecycle management with weak refs
//		LANARTS_ASSERT(!inst->destroyed);
		GameInst** lua_inst = (GameInst**) lua_newuserdata(L, sizeof(GameInst*));
		*lua_inst = inst;
		luameta_push(L, &ref_metatable);
		lua_setmetatable(L, -2);
		GameInst::retain_reference(inst);
	}

	void make_object_ref(LuaField table, GameInst* inst) {
		push_ref(table.luastate(), inst);
		table["__objectref"].pop();
		inst->lua_variables = table;
	}

	void push(lua_State* L, GameInst* inst) {
		if (inst == NULL) {
			lua_pushnil(L);
			return;
		}
		LuaValue& lua_vars = inst->lua_variables;
		if (lua_vars.empty()) {
			lua_vars = LuaValue::newtable(L);
		}
		if (lua_vars["__objectref"].isnil()) {
			make_object_ref(lua_vars, inst);
        }
		lua_vars.push();
		if (!lua_getmetatable(L, -1)) {
			lua_gameinst_push_metatable(L, inst);
			lua_setmetatable(L, -2);
		} else {
			lua_pop(L, 1);
		}
	}
}

typedef LuaValue (*exclusionf)(lua_State* L);

static void push_exclusion_set(lua_State* L, exclusionf func) {
	luawrap::registry(L).push();
	lua_pushlightuserdata(L, (void*) func);
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1)) {
		lua_replace(L, -2);
		return; // Return cached set
	}
	lua_pop(L, 1); // pop nil
	LuaValue exclusions = func(L);
	lua_pushlightuserdata(L, (void*) func);
	exclusions.push();
	lua_rawset(L, -3); // set cache
	exclusions.push(); // push value
}

static GameInst* copy_over_arguments(GameInst* inst, LuaStackValue args, exclusionf exclusions) {
	lua_State* L = args.luastate();
	int pre_top = lua_gettop(L);
	push_exclusion_set(L, exclusions);
	int ex_idx = lua_gettop(L);

	inst->lua_variables.init(L);
	inst->lua_variables.newtable();
	inst->lua_variables.push();
	int lv_idx = lua_gettop(L);
	lua_pushnil(L);
	while (lua_next(L, args.index())) {
		// Check exclusion set
		lua_pushvalue(L, -2); // Push key
		lua_rawget(L, ex_idx);
		if (!lua_isnil(L, -1)) {
			lua_pop(L, 2); // Pop '1' and value
			continue; // Excluded!
		}
		lua_pop(L, 1); // pop nil
		// Set value
		lua_pushvalue(L, -2); // Push key
		lua_pushvalue(L, -2); // Push value
		lua_rawset(L, lv_idx);
		lua_pop(L, 1); // pop value
	}
	lua_settop(L, pre_top);
	return inst;
}

static GameInst* initialize_object(GameState* gs, GameInst* inst, LuaStackValue args, exclusionf exclusions) {
	if (args["__nocopy"].isnil()) {
		copy_over_arguments(inst, args, exclusions);
	}

	if (!args["radius"].isnil()) {
		inst->target_radius = args["radius"].to_int();
		inst->radius = std::min(15.0f, inst->target_radius);
	}
	if (args["do_init"].isnil() || args["do_init"].to_bool()) {
		level_id id;
		if (args["map"].isnil()) {
			if (!gs->get_level()) {
				throw std::runtime_error("Attempt to initialize object with no active levels! Try using with do_init = false");
			}
			id = gs->game_world().get_current_level_id();
		} else {
			id = args["map"]["_id"].to_int();
		}
		if (id < 0) {
                    delete inst;
                    return nullptr;
                }
                gs->add_instance(id, inst);
	}
	return inst;
}
static LuaValue base_object_exclusions(lua_State* L) {
	LuaValue X = LuaValue::newtable(L);
	X["xy"] = 1, X["radius"] = 1, X["do_init"] = 1;
	X["solid"] = 1, X["depth"] = 1, X["map"] = 1;
	return X;
}

static LuaValue misc_exclusions(lua_State* L) {
	LuaValue X = base_object_exclusions(L);
	X["sprite"] = 1, X["frame"] = 1, X["type"] = 1;
	return X;
}

static GameInst* enemy_create(LuaStackValue args) {
	GameState* gs = lua_api::gamestate(args);
	int etype = get_enemy_by_name(args["type"].to_str());
	int team = luawrap::defaulted(args["team"], MONSTER_TEAM);
	Pos xy = args["xy"].as<Pos>();
	EnemyInst* inst = new EnemyInst(etype, xy.x, xy.y, team);
	return initialize_object(gs, inst, args, &misc_exclusions);
}

static GameInst* object_create(LuaStackValue args) {
	using namespace luawrap;
	Pos xy = args["xy"].as<Pos>();
	bool solid = defaulted(args["solid"], false);
	int radius = defaulted(args["radius"], 15);
	int depth = defaulted(args["depth"], 0);
	GameInst* inst = new GameInst(xy.x, xy.y, radius, solid, depth);
	return initialize_object(lua_api::gamestate(args), inst, args, &base_object_exclusions);
}

static int object_init(lua_State* L) {
	LuaStackValue obj(L, 1);
	Pos xy = luawrap::get<Pos>(L, 2);
	double radius = luawrap::get_defaulted(L, 3, TILE_SIZE / 2.0 - 1.0);
	bool solid = luawrap::get_defaulted(L, 4, false);
	int depth = luawrap::get_defaulted(L, 5, 0);

	GameInstWrap::make_object_ref(obj, new GameInst(xy.x, xy.y, radius, solid, depth));
	return 0;
}

static GameInst* player_create(LuaStackValue args) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(args);
	Pos xy = args["xy"].as<Pos>();
	int team = luawrap::defaulted(args["team"], PLAYER_TEAM);
	GameInst* inst = new PlayerInst(CombatStats(), -1, xy, team, false);
	gs->player_data().register_player(args["name"].to_str(), (PlayerInst*)inst, "", LuaValue(), true, -1);
	return initialize_object(gs, inst, args, &base_object_exclusions);
}

static void player_init(LuaStackValue obj, Pos xy, std::string name) {
	GameState* gs = lua_api::gamestate(obj);
	PlayerInst* inst = new PlayerInst(CombatStats(), -1, xy, PLAYER_TEAM, false);
	GameInstWrap::make_object_ref(obj, inst);
	gs->player_data().register_player(name, inst, "", LuaValue(), true, -1);
}

static GameInst* feature_create(LuaStackValue args) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(args);

	FeatureInst* inst = new FeatureInst(args["xy"].as<Pos>(), (FeatureInst::feature_t)args["type"].to_int(),
			defaulted(args["solid"], false),
			res::sprite_id(args["sprite"].to_str()),
			defaulted(args["depth"], (int)FeatureInst::DEPTH),
			defaulted(args["frame"], 0));
	return initialize_object(gs, inst, args, &misc_exclusions);
}

static GameInst* animation_create(LuaStackValue args) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(args);

	sprite_id spr_id = -1;
	if (!args["sprite"].isnil()) {
		spr_id = res::sprite_id(args["sprite"].to_str());
	}

	AnimatedInst* inst = new AnimatedInst(
			args["xy"].as<Pos>(),
			spr_id,
			defaulted(args["duration"], -1),
			defaulted(args["velocity"], PosF()),
			defaulted(args["orientation"], PosF()),
			defaulted(args["depth"], (int)AnimatedInst::DEPTH),
			defaulted(args["text"], std::string()),
			defaulted(args["text_color"], Colour()));
	return initialize_object(gs, inst, args, &base_object_exclusions);
}

static Item lua_item_get(LuaStackValue item) {
	item_id type = get_item_by_name(item["type"].to_str());
	int amount = luawrap::defaulted(item["amount"], 1);
	return Item(type, amount);
}

static GameInst* store_create(LuaStackValue args) {
	using namespace luawrap;
	lua_State* L = args.luastate();
	GameState* gs = lua_api::gamestate(args);

	// Determine inventory
	StoreInventory inventory;
	LuaField items = args["items"];
	int size = items.objlen();
	for (int i = 1; i <= size; i++) {
		items[i].push();
		LuaStackValue item(L, -1);
		item_id type = get_item_by_name(item["type"].to_str());
		ItemEntry& entry = get_item_entry(type);
		int amount = luawrap::defaulted(item["amount"], 1);
		int cost = gs->rng().rand(entry.shop_cost.multiply(amount));
		inventory.add(Item(type, amount), cost);
	}
	sprite_id spr_id = res::sprite_id(args["sprite"].to_str());

	StoreInst* inst = new StoreInst(args["xy"].as<Pos>(),
			defaulted(args["solid"], false),
			spr_id, inventory,
			defaulted(args["frame"], 0));
	return initialize_object(gs, inst, args, &base_object_exclusions);
}

static GameInst* item_create(LuaStackValue args) {
	return initialize_object(lua_api::gamestate(args),
			new ItemInst(lua_item_get(args), args["xy"].as<Pos>()), args,
			&base_object_exclusions);
}

static void object_destroy(LuaStackValue inst) {
	lua_api::gamestate(inst)->remove_instance(inst.as<GameInst*>());
}

static void object_add(LuaStackValue inst) {
    auto* gs = lua_api::gamestate(inst);
    if (!gs->get_level()) {
        throw std::runtime_error("Attempt to initialize object with no active levels! Try using with do_init = false");
    }
    gs->add_instance(inst.as<GameInst*>());
}

static const char* get_type(GameInst* inst) {
    if (dynamic_cast<ItemInst*>(inst)) {
        return "item";
    } else if (dynamic_cast<CombatGameInst*>(inst)) {
        return "actor";
    } else if (dynamic_cast<FeatureInst*>(inst)) {
        return "feature";
    } else if (dynamic_cast<StoreInst*>(inst)) {
        return "store";
    }
    return "<Unknown>";
}
namespace lua_api {
	// Ensures important objects are reached during serialization:
	static void ensure_reachability(LuaValue globals, LuaValue submodule) {
		lua_State* L = submodule.luastate();
		luameta_push(L, &lua_playerinst_metatable);
		submodule["PlayerType"].pop();
		luameta_push(L, &lua_enemyinst_metatable);
		submodule["EnemyType"].pop();
		luameta_push(L, &lua_combatgameinst_metatable);
		submodule["CombatType"].pop();
		luameta_push(L, &lua_feature_metatable);
		submodule["FeatureType"].pop();
        luameta_push(L, &lua_iteminst_metatable);
        submodule["ItemType"].pop();
        luameta_push(L, &lua_projectileinst_metatable);
        submodule["ProjectileType"].pop();
		luameta_push(L, &lua_gameinst_base_metatable);
		submodule["Base"].pop();
		luameta_push(L, &GameInstWrap::ref_metatable);
		submodule["REF_META"].pop();
	}
	void register_lua_core_GameObject(lua_State* L) {
		luawrap::install_type<GameInst*, GameInstWrap::push, GameInstWrap::get, GameInstWrap::check>();
		luawrap::install_dynamic_casted_type<CombatGameInst*, GameInst*>();
		luawrap::install_dynamic_casted_type<PlayerInst*, GameInst*>();
		luawrap::install_dynamic_casted_type<EnemyInst*, GameInst*>();
		luawrap::install_dynamic_casted_type<ItemInst*, GameInst*>();
		luawrap::install_dynamic_casted_type<FeatureInst*, GameInst*>();
		luawrap::install_dynamic_casted_type<ProjectileInst*, GameInst*>();

		LuaValue globals = luawrap::globals(L);
		LuaValue submodule = lua_api::register_lua_submodule(L, "core.GameObject");
		submodule["enemy_create"].bind_function(enemy_create);
		submodule["object_create"].bind_function(object_create);
		submodule["feature_create"].bind_function(feature_create);
		submodule["simulate_bounce"] = [L](GameInst* inst, PosF dir) -> PosF {
            GameState* gs = lua_api::gamestate(L);
            auto* old_level = gs->get_level();
            gs->set_level(inst->get_map(gs));
			int newx = (int) round(inst->x + dir.x); //update based on rounding of true float
			int newy = (int) round(inst->y + dir.y);
			bool collides = gs->tile_radius_test(newx, newy, inst->radius);
			bool hitsx = gs->tile_radius_test(newx, inst->y, inst->radius);
			bool hitsy = gs->tile_radius_test(inst->x, newy, inst->radius);
			if (hitsy || hitsx || collides) {
				if (hitsx) {
					dir.x = -dir.x;
				}
				if (hitsy) {
					dir.y = -dir.y;
				}
				if (!hitsy && !hitsx) {
					dir.x = -dir.x;
					dir.y = -dir.y;
				}
			}
            gs->set_level(old_level);
			return dir;
		};
		submodule["item_create"].bind_function(item_create);
		submodule["animation_create"].bind_function(animation_create);
		submodule["store_create"].bind_function(store_create);
                submodule["player_create"].bind_function(player_create);
                submodule["get_type"].bind_function(get_type);
		submodule["destroy"].bind_function(object_destroy);
		submodule["add_to_level"].bind_function(object_add);

		submodule["DOOR_OPEN"] = (int)FeatureInst::DOOR_OPEN;
		submodule["DOOR_CLOSED"] = (int)FeatureInst::DOOR_CLOSED;
		submodule["PORTAL"] = (int)FeatureInst::PORTAL;
		submodule["OTHER"] = (int)FeatureInst::OTHER;
		ensure_reachability(globals, submodule);
	}
}
