#ifndef LUA_CBIND_UTIL_H_
#define LUA_CBIND_UTIL_H_

#include <lua/lunar.h>

#include "lua_api.h"

template <class T>
class LuaCBindImpl : public  {
public:
	static const char className[];
	static Lunar<LuaCBindImpl>::RegType methods[];

	LuaCBindImpl(lua_State* L, obj_id id) :
			gs(lua_get_gamestate(L)), id(id) {
	}

	GameInst* get_instance() {
		return gs->get_instance(id);
	}

private:
	void* id;
};


#endif /* LUA_CBIND_UTIL_H_ */
