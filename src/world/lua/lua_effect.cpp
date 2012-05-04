#include "lua_api.h"
#include <cstring>

#include <lua/lunar.h>
#include "../GameState.h"
#include "../objects/EnemyInst.h"
#include "../objects/GameInst.h"
#include "../objects/PlayerInst.h"
#include "../../gamestats/Stats.h"

#include "../../data/item_data.h"
#include "../../data/effect_data.h"
#include "../../data/weapon_data.h"


class EffectsLuaBinding {
public:
  static const char className[];
  static Lunar<EffectsLuaBinding>::RegType methods[];

  EffectsLuaBinding(int effectnum) : effectnum(effectnum) {
  }

  EffectType& get_effects(){
	  return game_effect_data[effectnum];
  }
private:
  int effectnum;
};

typedef EffectsLuaBinding bind_t;
typedef Lunar<EffectsLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)


meth_t bind_t::methods[] = {
  meth_t(0,0)
};

void lua_pusheffects(lua_State* lua_state, int effectnum){
	lunar_t::push(lua_state, new EffectsLuaBinding(effectnum), true);
}

EffectType& lua_effects_arg(lua_State* lua_state, int narg){
	bind_t* bind = lunar_t::check(lua_state, narg);
	return bind->get_effects();
}

void lua_effects_bindings(GameState* gs, lua_State* lua_state){
	lunar_t::Register(lua_state);
     luaL_getmetatable(lua_state, bind_t::className);
     lua_newtable(lua_state);
     int top = lua_gettop(lua_state);
     for (int i = 0; i < game_effect_n; i++) {
    	    lua_pushstring(lua_state, game_effect_data[i].name);
    	    lua_pusheffects(lua_state, i);
    	    lua_settable(lua_state, top);
     }
     lua_setglobal(lua_state, "effects");
//

}

const char EffectsLuaBinding::className[] = "Effects";
