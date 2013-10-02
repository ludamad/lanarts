/*
 * lua_core_CollisionAvoidance.cpp:
 *	core.CollisionAvoidance submodule: Handles application of RVO algorithm for path-finding.
 */

#include <luawrap/luawrap.h>
#include <lcommon/geometry.h>
#include <lcommon/smartptr.h>

#include "lua_api.h"
#include "collision_avoidance/CollisionAvoidance.h"

typedef smartptr<CollisionAvoidance> CollisionAvoidancePtr;

static simul_id colavoid_add_object(CollisionAvoidancePtr colavoid, LuaStackValue args) {
	using namespace luawrap;
	return colavoid->add_active_object(args["xy"].as<PosF>(),
			args["radius"].to_num(), defaulted(args["speed"], 0.0f));
}

static void colavoid_step(CollisionAvoidancePtr colavoid) {
	colavoid->step();
}

static void colavoid_update_object(CollisionAvoidancePtr colavoid, simul_id id, LuaStackValue args) {
	using namespace luawrap;
	if (!args["speed"].isnil()) {
		colavoid->set_maxspeed(id, args["speed"].to_int());
	}
	if (!args["xy"].isnil()) {
		PosF xy = args["xy"].as<PosF>();
		colavoid->set_position(id, xy.x, xy.y);
	}
	if (!args["preferred_velocity"].isnil()) {
		PosF vxy = args["preferred_velocity"].as<PosF>();
		colavoid->set_preferred_velocity(id, vxy.x, vxy.y);
	}
}

static PosF colavoid_object_xy(CollisionAvoidancePtr colavoid, simul_id id) {
	return colavoid->get_position(id);
}

static void colavoid_object_copy_xy(CollisionAvoidancePtr colavoid, simul_id id, LuaStackValue args) {
	args["xy"] = colavoid->get_position(id);
}

LuaValue lua_colavoidmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "CollisionAvoidance");
	LuaValue methods = luameta_constants(meta);
	methods["add_object"].bind_function(colavoid_add_object);
	methods["update_object"].bind_function(colavoid_update_object);
	methods["object_xy"].bind_function(colavoid_object_xy);
	methods["object_copy_xy"].bind_function(colavoid_object_copy_xy);
	methods["step"].bind_function(colavoid_step);

	luameta_gc<CollisionAvoidance>(meta);

	return meta;
}

static CollisionAvoidancePtr colavoid_collision_group_create() {
	return CollisionAvoidancePtr(new CollisionAvoidance());
}

namespace lua_api {
	void register_lua_core_CollisionAvoidance(lua_State* L) {
		LuaValue colavoid = register_lua_submodule(L, "core.CollisionAvoidance");
		luawrap::install_userdata_type<CollisionAvoidancePtr, &lua_colavoidmetatable>();
		colavoid["collision_group_create"].bind_function(colavoid_collision_group_create);
	}
}
