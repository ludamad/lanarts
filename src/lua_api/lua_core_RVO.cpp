#include <cassert>

#include <rvo2/RVO.h>

#define LUAWRAP_LONG_FUNCTIONS
#include <luawrap/luawrap.h>

#include "lua_api.h"

using namespace RVO;

static int add_instance(RVOSimulator& sim,
        double x,
        double y,
        double radius, double maxSpeed) {

    const int MAX_NEIGHBOURS = 10;
    const float TIME_HORIZON = 2.0f, TIME_STEP = 1.0f;
    const float neighbour_dist = radius * 2;

    return sim.addAgent(Vector2(x, y), neighbour_dist, MAX_NEIGHBOURS,
            TIME_HORIZON, TIME_STEP, radius, maxSpeed);
}

static void remove_instance(RVOSimulator& sim, int id) {
    sim.removeAgent(id);
}

static void update_instance(RVOSimulator& sim,
        int id, double x, double y,
        double radius, double maxSpeed,
        double vx, double vy) {
    sim.setAgentPosition(id, Vector2(x,y));
    sim.setAgentRadius(id, radius);
    sim.setAgentMaxSpeed(id, maxSpeed);
    sim.setAgentPrefVelocity(id, Vector2(vx, vy));
}

static void change_instance_id(RVOSimulator& sim,
        int id, int new_id) {
    sim.changeAgentNo(id, new_id);
}

static int get_velocity(lua_State* L) {
    RVOSimulator* sim = luawrap::get<RVOSimulator*>(L, 1);
    Vector2 velocity = sim->getAgentVelocity(lua_tointeger(L, 2));
    lua_pushnumber(L, velocity.x());
    lua_pushnumber(L, velocity.y());
    return 2;
}

static int get_position(lua_State* L) {
    RVOSimulator* sim = luawrap::get<RVOSimulator*>(L, 1);
    Vector2 position = sim->getAgentPosition(lua_tointeger(L, 2));
    lua_pushnumber(L, position.x());
    lua_pushnumber(L, position.y());
    return 2;
}

static int get_radius(lua_State* L) {
    RVOSimulator* sim = luawrap::get<RVOSimulator*>(L, 1);
    lua_pushnumber(L, sim->getAgentRadius(lua_tointeger(L, 2)));
    return 1;
}

static int get_preferred_velocity(lua_State* L) {
    RVOSimulator* sim = luawrap::get<RVOSimulator*>(L, 1);
    Vector2 velocity = sim->getAgentPrefVelocity(lua_tointeger(L, 2));
    lua_pushnumber(L, velocity.x());
    lua_pushnumber(L, velocity.y());
    return 2;
}

static int set_position(lua_State* L) {
    RVOSimulator* sim = luawrap::get<RVOSimulator*>(L, 1);
    Vector2 position(lua_tonumber(L, 3), lua_tonumber(L, 4));
    sim->setAgentPosition(lua_tointeger(L, 2), position);
    return 0;
}

static int set_preferred_velocity(lua_State* L) {
    RVOSimulator* sim = luawrap::get<RVOSimulator*>(L, 1);
    Vector2 velocity(lua_tonumber(L, 3), lua_tonumber(L, 4));
    sim->setAgentPrefVelocity(lua_tointeger(L, 2), velocity);
    return 0;
}

static void step(RVOSimulator& sim) {
    sim.doStep();
}

static void clear(RVOSimulator& sim) {
    sim.~RVOSimulator();
    new (&sim) RVOSimulator();
}

LuaValue lua_rvoworldmetatable(lua_State* L) {
    LuaValue meta = luameta_new(L, "RVOWorld");
    LuaValue methods = luameta_constants(meta);

    methods["add_instance"].bind_function(add_instance);
    methods["remove_instance"].bind_function(remove_instance);
    methods["update_instance"].bind_function(update_instance);
    methods["get_velocity"].bind_function(get_velocity);
        methods["get_position"].bind_function(get_position);
        methods["set_position"].bind_function(set_position);
        methods["get_radius"].bind_function(get_radius);
    methods["get_preferred_velocity"].bind_function(get_preferred_velocity);
    methods["set_preferred_velocity"].bind_function(set_preferred_velocity);
    methods["change_instance_id"].bind_function(change_instance_id);
    methods["step"].bind_function(step);
    methods["clear"].bind_function(clear);

    luameta_gc<RVOSimulator>(meta);

    return meta;
}

static int new_rvo_world(lua_State* L) {
    int n_args = lua_gettop(L);
    void* valptr = luameta_newuserdata(L, lua_rvoworldmetatable, sizeof(RVOSimulator));
    RVOSimulator* sim = new (valptr) RVOSimulator();

    if (n_args >= 1 && !lua_isnil(L, 1)) {
        LuaValue obstacles(L, 1);
        for (int i = 1; i <= obstacles.objlen(); i++) {
            LuaValue obstacle = obstacles[i];
            std::vector<Vector2> vec;
            for (int i = 1; i <= obstacle.objlen(); i++) {
                LuaValue point = obstacle[i];
                vec.push_back(Vector2(point[1].to_num(), point[2].to_num()));
            }
            sim->addObstacle(vec);
        }
        // Note: This may take long!
        sim->processObstacles();
    }
    return 1;
}

namespace lua_api {
    void register_lua_core_RVOWorld(lua_State* L) {
            LuaValue module = register_lua_submodule(L, "core.RVOWorld");
            luawrap::install_userdata_type<RVOSimulator, &lua_rvoworldmetatable>();
            module["create"].bind_function(new_rvo_world);
    }
}
