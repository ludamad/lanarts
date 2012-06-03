#include <cstring>
#include <lua/lunar.h>

#include "lua_api.h"

#include "../world/GameState.h"
#include "../world/objects/EnemyInst.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"
#include "../gamestats/Stats.h"

#include "../data/item_data.h"
#include "../data/effect_data.h"
#include "../data/weapon_data.h"


class EffectLuaBinding {
public:
  static const char className[];
  static Lunar<EffectLuaBinding>::RegType methods[];

  EffectLuaBinding(int effectnum) : effectnum(effectnum) {
  }

  EffectEntry& get_effects(){
	  return game_effect_data[effectnum];
  }
private:
  obj_id id;
  int effectnum;
};

typedef EffectLuaBinding bind_t;
typedef Lunar<EffectLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)


meth_t bind_t::methods[] = {
  meth_t(0,0)
};

void lua_pusheffects(lua_State* L, int effectnum){
	lunar_t::push(L, new EffectLuaBinding(effectnum), true);
}

EffectEntry& lua_effects_arg(lua_State* L, int narg){
	bind_t* bind = lunar_t::check(L, narg);
	return bind->get_effects();
}

void lua_effects_bindings(GameState* gs, lua_State* L){
	lunar_t::Register(L);
     luaL_getmetatable(L, bind_t::className);
     lua_newtable(L);
     int top = lua_gettop(L);
     for (int i = 0; i < game_effect_data.size(); i++) {
    	    lua_pushstring(L, game_effect_data[i].name.c_str());
    	    lua_pusheffects(L, i);
    	    lua_settable(L, top);
     }
     lua_setglobal(L, "effects");
//

}

const char EffectLuaBinding::className[] = "Effects";
