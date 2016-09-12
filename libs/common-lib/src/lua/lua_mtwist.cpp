/*
 * lua_mtwist.cpp:
 *  Registers the MTwist class for lua.
 */

#include <cassert>
#include <cmath>

#include <luawrap/LuaValue.h>
#include <luawrap/luameta.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include "mtwist.h"

std::string _luatrace();

static int random(lua_State* L) {
    int n_args = lua_gettop(L);

    if (n_args == 0 || n_args > 3) {
        return luaL_error(L, "Expected the MTwist object followed by 0 to 2 arguments! Got '%d' total.", n_args);
    }

    int num = -1;
    if (n_args == 1) {
        num = luawrap::get<MTwist*>(L, 1)->rand();
    } else if (n_args == 2) {
        num = luawrap::get<MTwist*>(L, 1)->rand(lua_tointeger(L, 2));
    } else if (n_args == 3) {
        num = luawrap::get<MTwist*>(L, 1)->rand(lua_tointeger(L, 2), lua_tointeger(L, 3));
    }
    lua_pushinteger(L, num);
    return 1;
}


static int randomf(lua_State* L) {
    int n_args = lua_gettop(L);

    if (n_args == 0 || n_args > 3) {
        return luaL_error(L, "Expected the MTwist object followed by 0 to 2 arguments! Got '%d' total.", n_args);
    }

    double num = -1;
    if (n_args == 1) {
        num = luawrap::get<MTwist*>(L, 1)->randf();
    } else if (n_args == 2) {
        RangeF range(0, lua_tonumber(L, 2));
        num = luawrap::get<MTwist*>(L, 1)->rand(range);
    } else if (n_args == 3) {
        RangeF range(lua_tonumber(L, 2), lua_tonumber(L, 3));
        num = luawrap::get<MTwist*>(L, 1)->rand(range);
    }
    lua_pushnumber(L, num);
    return 1;
}

static double random_round(MTwist* mtwist, double val) {
    double f = floor(val);
    if (mtwist->randf() > (val - f)) {
        return f + 1;
    } else {
        return f;
    }
}

static double guassian(MTwist* mtwist, double average, double std_dev, int n_trials) {
    return mtwist->guassian(average, std_dev, n_trials);
}

static int amount_generated(MTwist* mtwist) {
    return mtwist->amount_generated();
}

LuaValue lua_mtwistmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "MTwist");
	LuaValue methods = luameta_constants(meta);

	methods["random"].bind_function(random);
	methods["randomf"].bind_function(randomf);
	methods["random_round"].bind_function(random_round);
	methods["guassian"].bind_function(guassian);
	methods["amount_generated"].bind_function(amount_generated);

	luameta_gc<MTwist>(meta);

	return meta;
}

static MTwist newmtwist(unsigned int seed) {
	return MTwist(seed);
}

int luaopen_mtwist(lua_State *L) {
	luawrap::install_userdata_type<MTwist, &lua_mtwistmetatable>();
	LuaValue module = LuaValue::newtable(L);
	module["create"].bind_function(newmtwist);
	module.push();
	return 1;
}
