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

  ItemLuaBinding(ItemEntry& item) : item(item) {
  }

  ItemEntry& get_item(){
	  return item;
  }
private:
  ItemEntry& item;
};

typedef ItemLuaBinding bind_t;
typedef Lunar<ItemLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)


meth_t bind_t::methods[] = {
  meth_t(0,0)
};

void lua_pushitem(lua_State* L, ItemEntry& item){
	lunar_t::push(L, new ItemLuaBinding(item), true);
}

ItemEntry& lua_item_arg(lua_State* L, int narg){
	bind_t* bind = lunar_t::check(L, narg);
	return bind->get_item();
}
static int lua_member_lookup(lua_State* L){
	#define IFLUA_STR_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushstring(L, m );\
	}
	#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}
	#define IFLUA_BOOL_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushboolean(L, m );\
	}

	bind_t* state = lunar_t::check(L,1);
	ItemEntry& item = state->get_item();
	bool is_weapon = item.weapon != 0;
	WeaponEntry* weap = NULL;
	if (is_weapon) weap = &game_weapon_data[item.weapon];
	const char* cstr = lua_tostring(L, 2);

	if (is_weapon){
		IFLUA_STR_MEMB_LOOKUP("weapon_name", weap->name)
		else IFLUA_NUM_MEMB_LOOKUP("projectile_speed", weap->projectile_speed)
	}
    IFLUA_BOOL_MEMB_LOOKUP("is_weapon", item.weapon == 0)
			else IFLUA_STR_MEMB_LOOKUP("item_name", item.name)
	else{
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}


void lua_item_bindings(GameState* gs, lua_State* L){
	lunar_t::Register(L);

     luaL_getmetatable(L, bind_t::className);
//
    int tableind = lua_gettop(L);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_member_lookup);
    lua_settable(L, tableind);
}

const char ItemLuaBinding::className[] = "Item";
