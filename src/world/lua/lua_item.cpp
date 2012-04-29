#include "lua_api.h"
#include <cstring>

#include <lua/lunar.h>
#include "../GameState.h"
#include "../objects/EnemyInst.h"
#include "../objects/GameInst.h"
#include "../objects/PlayerInst.h"
#include "../../gamestats/Stats.h"

#include "../../data/item_data.h"
#include "../../data/weapon_data.h"


class ItemLuaBinding {
public:
  static const char className[];
  static Lunar<ItemLuaBinding>::RegType methods[];

  ItemLuaBinding(ItemType& item) : item(item) {
  }

  ItemType& get_item(){
	  return item;
  }
private:
  ItemType& item;
};

typedef ItemLuaBinding bind_t;
typedef Lunar<ItemLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)


meth_t bind_t::methods[] = {
  meth_t(0,0)
};

void lua_pushitem(lua_State* lua_state, ItemType& item){
	lunar_t::push(lua_state, new ItemLuaBinding(item), true);
}

ItemType& lua_item_arg(lua_State* lua_state, int narg){
	bind_t* bind = lunar_t::check(lua_state, narg);
	return bind->get_item();
}
static int lua_member_lookup(lua_State* lua_state){
	#define IFLUA_STR_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushstring(lua_state, m );\
	}
	#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(lua_state, m );\
	}
	#define IFLUA_BOOL_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushboolean(lua_state, m );\
	}

	bind_t* state = lunar_t::check(lua_state,1);
	ItemType& item = state->get_item();
	bool is_weapon = item.weapon != 0;
	WeaponType* weap = NULL;
	if (is_weapon) weap = &game_weapon_data[item.weapon];
	const char* cstr = lua_tostring(lua_state, 2);

	if (is_weapon){
		IFLUA_STR_MEMB_LOOKUP("weapon_name", weap->name)
		else IFLUA_NUM_MEMB_LOOKUP("projectile_speed", weap->projectile_speed)
	}
    IFLUA_BOOL_MEMB_LOOKUP("is_weapon", item.weapon == 0)
			else IFLUA_STR_MEMB_LOOKUP("item_name", item.name)
			else IFLUA_NUM_MEMB_LOOKUP("action_amount", item.action_amount)
			else IFLUA_NUM_MEMB_LOOKUP("action_duration", item.action_duration)
	else{
		lua_getglobal(lua_state, bind_t::className);
		int tableind = lua_gettop(lua_state);
		lua_pushvalue(lua_state, 2);
		lua_gettable(lua_state, tableind);
	}
	return 1;
}


void lua_item_bindings(GameState* gs, lua_State* lua_state){
	lunar_t::Register(lua_state);

     luaL_getmetatable(lua_state, bind_t::className);
//
    int tableind = lua_gettop(lua_state);

    lua_pushstring(lua_state, "__index");
    lua_pushcfunction(lua_state, lua_member_lookup);
    lua_settable(lua_state, tableind);
}

const char ItemLuaBinding::className[] = "Item";
