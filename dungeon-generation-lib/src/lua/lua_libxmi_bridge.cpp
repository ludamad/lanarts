/*
 * map_fill.cpp:
 *  Various algorithms for filling a map.
 */

#include <cstdlib>
#include <algorithm>
#include <vector>

extern "C" {
	#include <xmi.h>
}

#include "lua_ldungeon_impl.h"
#include "../ldungeon_assert.h"

#include <luawrap/luawrap.h>
#include <luawrap/macros.h>
#include <lcommon/smartptr.h>

#include "map_fill.h"

// Convenience
typedef std::vector<ldungeon_gen::ConditionalOperator> OperatorTable;

/*********************************************************************
 ********          Internal LibXMI fiddling                    *******
 *********************************************************************/

// We must expose libxmi internals to efficiently draw on our custom map format.
// Additionally, we must copy the struct definitions due to problems with including libxmi headers in C++ code.
typedef struct {
	int count;
	miPoint *points;
	unsigned int *widths;
} Spans;

typedef struct {
	miPixel pixel;
	Spans *group;
	int size;
	int count;
	int ymin, ymax;
} SpanGroup;

typedef struct lib_miPaintedSet {
	SpanGroup **groups;
	int size;
	int ngroups;
} _miPaintedSet;

// Define operators that traverse our custom map format using LibXMI's structures
static inline void apply_operator_to_lines(ldungeon_gen::Map& map, /*bounds*/
		BBox B, ldungeon_gen::ConditionalOperator oper, int n,
		const miPoint *ppt, const unsigned int *pwidth, Pos offset) {

	/* if source doesn't overlap with destination drawable, do nothing */
	if (ppt[0].y + offset.y >= B.y2 || ppt[n - 1].y + offset.y < B.y1) {
		return;
	}

	for (int i = 0; i < n; i++) {
		int y = ppt[i].y + offset.x;
		if (y >= B.y2) {
			return;
		}
		if (y >= B.y1) {
			int xstart = ppt[i].x + offset.x;
			int xend = xstart + (int) pwidth[i] - 1;

			int xstart_clip = std::max(xstart, B.x1);
			int xend_clip = std::min(xend, B.x2 - 1);

			for (int x = xstart_clip; x <= xend_clip; x++) {
				map[Pos(x, y)].apply(oper);
			}
		}
	}
}

static void apply_operators_to_painted_set(const miPaintedSet *paintedSet,
		ldungeon_gen::Map& map, const OperatorTable& operator_table, BBox B, Pos offset) {
	/* For each pixel color, the initial Spans in the corresponding SpanGroup
	 is effectively a list of spans, in y-increasing order.  That is, it's
	 a list of starting points and a corresponding list of widths; both are
     of length `count'. */

	for (int i = 0; i < paintedSet->ngroups; i++) {
		SpanGroup* group = paintedSet->groups[i];
		Spans* spans = &group->group[0];
		if (spans->count > 0) {
			LDUNGEON_ASSERT(group->pixel >= 1 && group->pixel <= operator_table.size());
			apply_operator_to_lines(map, B, operator_table[group->pixel - 1
			                                               ],
					spans->count, spans->points, spans->widths, offset);
		}
	}
}


static miGC* init_gc() {
	miPixel pixels[] = {1,2};
	unsigned int dashes[2] = {4, 2};
	miGC* gc = miNewGC(2, pixels);
//	miSetGCAttrib(gc, MI_GC_LINE_STYLE, MI_LINE_ON_OFF_DASH);
//	miSetGCDashes(gc, 2, dashes, 0);
	miSetGCAttrib(gc, MI_GC_LINE_WIDTH, 2); /* Bresenham algorithm */

    return gc;
}

/*********************************************************************
 ********          LibXMI wrapper functions                    *******
 *********************************************************************/

struct PaintedSetWrapper {
	PaintedSetWrapper() {
		set = miNewPaintedSet ();
		next_index = 1;
		gc = init_gc();
	}
	~PaintedSetWrapper() {
		miDeletePaintedSet(set);
	}

	int index(LuaStackValue identifier) {
		lua_State* L = identifier.luastate();
		ensure_index_map(L);

		index_map.push();
		identifier.push();
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1)) {
			int result = lua_tointeger(L, -1);
			lua_pop(L, 2);
			return result;
		}
		// Grab next available index
		int result = (next_index++);

		// Cache it
		lua_pop(L, 1); // Pop nil
		identifier.push();
		lua_pushinteger(L, result);
		lua_rawset(L, -3);
		lua_pop(L, 1); // Pop index_map


	}

	void set_pixel_fill(miPixel value) {
		miPixel pixels[] = {1,value};
		miSetGCPixels(gc, sizeof(pixels)/sizeof(miPixel), pixels);
	}

	void ensure_index_map(lua_State* L) {
		if (index_map.empty()) {
			index_map.init(L);
			index_map.newtable();
		}
	}

	miPaintedSet* set;
	LuaValue index_map;
	int next_index;
	miGC* gc;
};

typedef smartptr<PaintedSetWrapper> PaintedSetWrapperPtr;

namespace ldungeon_gen {

	typedef std::vector<Pos> Polygon;

	static PaintedSetWrapperPtr shape_set_create() {
		return PaintedSetWrapperPtr(new PaintedSetWrapper());
	}

	static void polygon_render(PaintedSetWrapperPtr ps, const Polygon& points, LuaStackValue identifier) {
		ps->set_pixel_fill(ps->index(identifier));
		miFillPolygon(ps->set, ps->gc, MI_SHAPE_GENERAL, MI_COORD_MODE_ORIGIN, points.size(), (miPoint*)&points[0]);
	}

	static int polygon_apply(lua_State* L) {
		MapPtr map = luawrap::get<MapPtr>(L, 1);
		ConditionalOperator oper = lua_conditional_operator_get(LuaStackValue(L, 2));
		const Polygon& points = luawrap::get<Polygon>(L, 3);
		Pos offset = lua_gettop(L) >= 4 ? luawrap::get<Pos>(L, 4) : Pos();
		BBox area = lua_gettop(L) >= 5 ? luawrap::get<BBox>(L, 5) : BBox(Pos(), map->size());

		PaintedSetWrapper ps;
		OperatorTable operators(1);
		operators[0] = oper;

		ps.set_pixel_fill(1);
		miFillPolygon(ps.set, ps.gc, MI_SHAPE_GENERAL, MI_COORD_MODE_ORIGIN, points.size(), (miPoint*)&points[0]);
		apply_operators_to_painted_set(ps.set, *map, operators, area, offset);
		return 0;
	}

	static void shape_set_apply(LuaStackValue args) {
		PaintedSetWrapperPtr ptr = args["shape_set"].as<PaintedSetWrapperPtr>();
		MapPtr map = args["map"].as<MapPtr>();
		BBox bounds = luawrap::defaulted(args["bounds"], BBox(Pos(), map->size()));
		Pos xy = args["xy"].as<Pos>();

		OperatorTable operators(ptr->next_index);
		lua_State* L = args.luastate();

		args["legend"].push();
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			LuaStackValue key(L, -2);
			LuaStackValue value(L, -1);

			ConditionalOperator op = lua_conditional_operator_get(value);
			operators.at(ptr->index(key) - 1) = op;

			lua_pop(L, 1);
		}
		lua_pop(L, 1); // table

		apply_operators_to_painted_set(ptr->set, *map, operators, bounds, xy);
	}

	LuaValue paintedset_metatable(lua_State* L) {
		LuaValue meta = luameta_new(L, "MapGen.ShapeSet");
		luameta_gc<PaintedSetWrapperPtr>(meta);
		return meta;
	}

	void register_libxmi_bindings(LuaValue submodule) {
		luawrap::install_userdata_type<PaintedSetWrapperPtr, paintedset_metatable>();
		submodule["shape_set_create"].bind_function(shape_set_create);
		submodule["shape_set_apply"].bind_function(shape_set_apply);
		submodule["polygon_render"].bind_function(polygon_render);
		submodule["polygon_apply"].bind_function(polygon_apply);
	}
}
