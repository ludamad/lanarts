#include "lua_api.h"

#include <lua/lunar.h>
#include "../GameState.h"
#include "../GameLevelState.h"

#include "../objects/EnemyInst.h"
#include "../objects/PlayerInst.h"
#include "../objects/ProjectileInst.h"

static Stats* get_stats(GameInst* inst) {
	PlayerInst* p;
	EnemyInst* e;
	if ((p = dynamic_cast<PlayerInst*>(inst))) {
		return &p->stats();
	} else if ((e = dynamic_cast<EnemyInst*>(inst))) {
		return &e->stats();
	} else {
		return NULL;
	}
}

class GameStateLuaBinding {
public:
	static const char className[];
	static Lunar<GameStateLuaBinding>::RegType methods[];

	GameStateLuaBinding(GameState* gs) :
			gs(gs) {
	}

	/*Takes GameInst object*/
	int remove_object(lua_State* L) {
		obj_id id = lua_gameinst_arg(L, 1);
		if (id > 0) {
			GameInst* inst = gs->get_instance(id);
			if (inst)
				gs->remove_instance(inst);
		}
		return 0;
	}
	/*Takes <object type> <sprite> <x> <y>, [<target x>, <target y>] */
	int create_projectile(lua_State* L) {
		int nargs = lua_gettop(L);
		obj_id origin_id = lua_gameinst_arg(L, 1);
		int tx = lua_tonumber(L, 2), ty = lua_tonumber(L, 3);//TODO
		bool bounce = nargs < 4 ? false : lua_toboolean(L, 4);//TODO
		int hits = nargs < 5 ? 1 : lua_tonumber(L, 5);//TODO
		obj_id target = nargs < 6 ? 0 : lua_gameinst_arg(L, 6);

		obj_id projectile_id = 0;
		GameInst* origin_obj = gs->get_instance(origin_id);
		Stats* s = get_stats(origin_obj);
		if (s != NULL) {
			Attack& atk = s->magicatk;
			GameInst* inst = new ProjectileInst(atk.attack_sprite, origin_id, atk.projectile_speed,
					atk.range, atk.damage, origin_obj->x, origin_obj->y, tx, ty, bounce,
					hits, target);
			projectile_id = gs->add_instance(inst);
		}

		lua_pushgameinst(L, projectile_id);
		return 0;
	}

	int players_in_room(lua_State* L) {
		PlayerController& pc = gs->player_controller();
		lua_createtable(L, 0, 0);
		int table = lua_gettop(L);
		for (int i = 0; i < pc.player_ids().size(); i++) {
			lua_pushgameinst(L, pc.player_ids()[i]);
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
#define IFLUA_NUM_MEMB_UPDATE(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}

	bind_t* state = lunar_t::check(L, 1);
	GameState* gs = state->game_state();
	const char* cstr = lua_tostring(L, 2);

	IFLUA_NUM_MEMB_UPDATE("width", gs->width())
	else IFLUA_NUM_MEMB_UPDATE("height", gs->height())
	else IFLUA_NUM_MEMB_UPDATE("mouse_x", gs->mouse_x())
	else IFLUA_NUM_MEMB_UPDATE("mouse_y", gs->mouse_y())
	else IFLUA_NUM_MEMB_UPDATE("frame_number", gs->frame())
	else IFLUA_NUM_MEMB_UPDATE("level_number", gs->level()->level_number)
	else IFLUA_NUM_MEMB_UPDATE("monster_num", gs->monster_controller().number_monsters())
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
