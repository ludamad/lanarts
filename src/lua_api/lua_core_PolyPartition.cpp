/*
 * lua_core_PolyPartition.cpp:
 *  Field-of-view submodule
 */

#include <SDL.h>

#include <ldungeon_gen/Map.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_api.h"

#include "lua_api/lua_api.h"
#include "fov/fov.h"

#include <lcommon/math_util.h>

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"


#include <polypartition/polypartition.h>

typedef std::vector<PosF> _Polygon;

static std::vector<_Polygon> decompose(const _Polygon& polygons) {
    TPPLPartition pp;
    TPPLPoly tp_poly;
    TPPLPolyList tp_parts;
    tp_poly.Init(polygons.size());
    for (size_t i = 0; i < polygons.size(); i++) {
        tp_poly[i] = {polygons[i].x, polygons[i].y, i};
    }
    tp_poly.SetOrientation(TPPL_CCW);
    pp.ConvexPartition_HM(&tp_poly, &tp_parts);
    std::vector<_Polygon> ret;
    for (TPPLPoly& tp_poly : tp_parts) {
        _Polygon polygon;
        for (size_t i = 0; i < tp_poly.GetNumPoints(); i++) {
            auto point = tp_poly.GetPoint(i);
            polygon.push_back({point.x, point.y});
        }
        ret.push_back(polygon);
    }
    return ret;
}

namespace lua_api {
    void register_lua_core_PolyPartition(lua_State* L) {
        LuaValue poly_part = register_lua_submodule(L, "core.PolyPartition");
        poly_part["decompose"].bind_function(decompose);
    }
}
