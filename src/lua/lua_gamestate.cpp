#include <lua/lunar.h>

#include "lua_api.h"

#include "../data/lua_game_data.h"

#include "../gamestate/GameState.h"
#include "../gamestate/GameLevelState.h"

#include "../objects/enemy/EnemyInst.h"
#include "../objects/player/PlayerInst.h"
#include "../objects/ProjectileInst.h"

class GameStateLuaBinding {
public:
	static const char className[];
	static Lunar<GameStateLuaBinding>::RegType methods[];

	GameStateLuaBinding(GameState* gs) :
			gs(gs) {
	}

	/*Takes GameInst object*/
	int remove_object(lua_State* L) {
		//TODO: Ensure correct level
		gs->remove_instance(lua_gameinst_arg(L, 1));
		return 0;
	}
	int create_projectile(lua_State* L) {
		//<origin>, <sprite>, <tx>, <ty> [, <target>]
		int nargs = lua_gettop(L);
		GameInst* origin_obj = lua_gameinst_arg(L, 1);

		sprite_id sprite = get_sprite_by_name(lua_tostring(L, lua_gettop(L)));
		lua_pop(L, 1);

		bool bounce = nargs < 8 ? false : lua_toboolean(L, 8);
		int hits = nargs < 9 ? 1 : lua_tonumber(L, 9);
		GameInst* target = nargs < 10 ? NULL : lua_gameinst_arg(L, 10);

		obj_id projectile_id = 0;



//		if (s != NULL) {
////			ProjectileInst(sprite_id sprite, obj_id originator, float speed, int range,
////					int damage, int x, int y, int tx, int ty, bool bounce = false,
////					int hits = 1, obj_id target = NONE);
//			GameInst* inst = new ProjectileInst(sprite, origin_id, lua_tonumber(L, 5),
//					lua_tonumber(L, 6), lua_tonumber(L, 7), origin_obj->x, origin_obj->y,
//					lua_tonumber(L, 2), lua_tonumber(L, 3), bounce,
//					hits, target);
//			projectile_id = gs->add_instance(inst);
//		}

//		lua_push_gameinst(L, inst);
		return 0;
	}

	int players_in_room(lua_State* L) {
		PlayerController& pc = gs->player_controller();
		lua_createtable(L, 0, 0);
		int table = lua_gettop(L);
		for (int i = 0; i < pc.player_ids().size(); i++) {
			lua_push_gameinst(L, gs->get_instance(pc.player_ids()[i]));
			//  lua_pushnumber(L, 2);
			lua_rawseti(L, table, i + 1);
		}
		return 1;
	}

	GameState* game_state() {
		return gs;
	}
private:
	GameState* gs;
};

typedef GameStateLuaBinding bind_t;
typedef Lunar<GameStateLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

static int lua_member_lookup(lua_State* L) {
#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}

	bind_t* state = lunar_t::check(L, 1);
	GameState* gs = state->game_state();
	const char* cstr = lua_tostring(L, 2);
	GameView& view = gs->view();

	IFLUA_NUM_MEMB_LOOKUP("width", gs->width())
	else IFLUA_NUM_MEMB_LOOKUP("height", gs->height())
	else IFLUA_NUM_MEMB_LOOKUP("mouse_x", gs->mouse_x() + view.x)
	else IFLUA_NUM_MEMB_LOOKUP("mouse_y", gs->mouse_y() + view.y)
	else IFLUA_NUM_MEMB_LOOKUP("frame_number", gs->frame())
	else IFLUA_NUM_MEMB_LOOKUP("level_number", gs->get_level()->level_number)
	else IFLUA_NUM_MEMB_LOOKUP("monster_num", gs->monster_controller().number_monsters())
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}

meth_t bind_t::methods[] = { LUA_DEF(remove_object), LUA_DEF(create_projectile),
		LUA_DEF(players_in_room), meth_t(0, 0) };

GameState* lua_get_gamestate(lua_State* L) {
	lua_getglobal(L, "world");
	int idx = lua_gettop(L);
	bind_t* obj = lunar_t::check(L, idx);
	lua_pop(L, 1);
	if (!obj)
		return NULL;
	return obj->game_state();
}

void lua_gamestate_bindings(GameState* gs, lua_State* L) {
	lunar_t::Register(L);

	luaL_getmetatable(L, bind_t::className);
//
	int tableind = lua_gettop(L);

	lua_pushstring(L, "__index");
	lua_pushcfunction(L, lua_member_lookup);
	lua_settable(L, tableind);

	lunar_t::push(L, new bind_t(gs), true);

	lua_setglobal(L, "world");
}
const char GameStateLuaBinding::className[] = "GameWorld";
