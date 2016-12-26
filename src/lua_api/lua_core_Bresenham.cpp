#include <cassert>

#include <rvo2/RVO.h>

#define LUAWRAP_LONG_FUNCTIONS
#include <luawrap/luawrap.h>

#include "lua_api.h"

#include <vector>
#include "util/bresenham.h"
#include <lcommon/geometry.h>

typedef bool (*TCOD_line_listener_t) (int x, int y);

static std::vector<Pos> LINE_BUFFER;

static bool bresenham_listener(int x, int y) {
    LINE_BUFFER.push_back(Pos {x, y});
    return true;
}

static std::vector<Pos> line_evaluate(Pos from, Pos to) {
    LINE_BUFFER.clear();
    TCOD_line(from.x, from.y, to.x, to.y, bresenham_listener);
    return LINE_BUFFER;
}
//TCOD_line

namespace lua_api {
    void register_lua_core_Bresenham(lua_State* L) {
            LuaValue module = register_lua_submodule(L, "core.Bresenham");
            module["line_evaluate"].bind_function(line_evaluate);
    }
}
