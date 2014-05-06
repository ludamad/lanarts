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
typedef std::vector<ldungeon_gen::Selector> SelectorTable;

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
static inline bool apply_operation_to_lines(ldungeon_gen::Map& map, /*bounds*/ BBox B,
		bool use_operator,
		ldungeon_gen::ConditionalOperator oper, ldungeon_gen::Selector selector,
		int n, const miPoint *ppt, const unsigned int *pwidth, Pos offset) {

	/* if source doesn't overlap with destination drawable, do nothing */
	if (ppt[0].y + offset.y >= B.y2 || ppt[n - 1].y + offset.y < B.y1) {
		return false;
	}

	for (int i = 0; i < n; i++) {
		int y = ppt[i].y + offset.x;
		if (y >= B.y2) {
			return false;
		}
		if (y >= B.y1) {
			int xstart = ppt[i].x + offset.x;
			int xend = xstart + (int) pwidth[i] - 1;

			int xstart_clip = std::max(xstart, B.x1);
			int xend_clip = std::min(xend, B.x2 - 1);

			if (use_operator) {
				for (int x = xstart_clip; x <= xend_clip; x++) {
					map[Pos(x, y)].apply(oper);
				}
			} else {
				// Selector
				for (int x = xstart_clip; x <= xend_clip; x++) {
					if (!map[Pos(x, y)].matches(selector)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

static void apply_operators_to_painted_set(const miPaintedSet *paintedSet,
		ldungeon_gen::Map& map, const OperatorTable& operator_table, BBox B, Pos offset) {
	for (int i = 0; i < paintedSet->ngroups; i++) {
		SpanGroup* group = paintedSet->groups[i];
		Spans* spans = &group->group[0];
		if (spans->count > 0) {
			LDUNGEON_ASSERT(group->pixel >= 1 && group->pixel <= operator_table.size());
			apply_operation_to_lines(map, B,
					true,
					operator_table[group->pixel - 1], ldungeon_gen::Selector(),
					spans->count, spans->points, spans->widths, offset);
		}
	}
}

static bool apply_selectors_to_painted_set(const miPaintedSet *paintedSet,
		ldungeon_gen::Map& map, const SelectorTable& selector_table, BBox B, Pos offset) {
	for (int i = 0; i < paintedSet->ngroups; i++) {
		SpanGroup* group = paintedSet->groups[i];
		Spans* spans = &group->group[0];
		if (spans->count > 0) {
			LDUNGEON_ASSERT(group->pixel >= 1 && group->pixel <= selector_table.size());
			bool matches = apply_operation_to_lines(map, B,
					false,
					ldungeon_gen::ConditionalOperator(), selector_table[group->pixel - 1],
					spans->count, spans->points, spans->widths, offset);
			if (!matches) {
				return false;
			}
		}
	}
	return true;
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

	typedef void applyf(LuaStackValue args, PaintedSetWrapper& ps);

	static void polygonf(LuaStackValue args, PaintedSetWrapper& ps) {
		const Polygon& points = args["points"].as<Polygon>();
		miFillPolygon(ps.set, ps.gc, MI_SHAPE_GENERAL, MI_COORD_MODE_ORIGIN, points.size(), (miPoint*)&points[0]);
	}

	static void linef(LuaStackValue args, PaintedSetWrapper& ps) {
		Pos start = args["from_xy"].as<Pos>();
		Pos end = args["to_xy"].as<Pos>();
		miPoint arr[] = { { start.x, start.y }, { end.x, end.y } };

		miDrawLines(ps.set, ps.gc, MI_COORD_MODE_ORIGIN, sizeof(arr)/sizeof(miPoint), arr);
	}

	static void configure(LuaStackValue args, PaintedSetWrapper& ps) {
		int line_width = luawrap::defaulted(args["line_width"], 1);
		miSetGCAttrib(ps.gc, MI_GC_LINE_WIDTH, line_width);
	}

	static void generic_apply(LuaStackValue args, applyf func) {
		lua_State* L = args.luastate();
		MapPtr map = args["map"].as<MapPtr>();

		PaintedSetWrapper ps;
		ps.set_pixel_fill(1);

		configure(args, ps);
		func(args, ps);

		ConditionalOperator oper = lua_conditional_operator_get(args["operator"]);
		OperatorTable operators(1);
		operators[0] = oper;
		Pos offset = luawrap::defaulted(args["xy"], Pos());
		BBox area = luawrap::defaulted(args["area"], BBox(Pos(), map->size()));
		apply_operators_to_painted_set(ps.set, *map, operators, area, offset);
	}

	static bool generic_query(LuaStackValue args, applyf func) {
		lua_State* L = args.luastate();
		MapPtr map = args["map"].as<MapPtr>();

		PaintedSetWrapper ps;
		ps.set_pixel_fill(1);

		configure(args, ps);
		func(args, ps);

		Selector selector = lua_selector_get(args["selector"]);
		SelectorTable selectors(1);
		selectors[0] = selector;
		Pos offset = luawrap::defaulted(args["xy"], Pos());
		BBox area = luawrap::defaulted(args["area"], BBox(Pos(), map->size()));
		return apply_selectors_to_painted_set(ps.set, *map, selectors, area, offset);
	}

	static void generic_render(LuaStackValue args, applyf func) {
		lua_State* L = args.luastate();
		PaintedSetWrapperPtr ps  = args["shape_set"].as<PaintedSetWrapperPtr>();
		ps->set_pixel_fill(1);
		func(args, *ps);
	}

	static void polygon_apply(LuaStackValue args) {
		generic_apply(args, polygonf);
	}

	static void line_apply(LuaStackValue args) {
		generic_apply(args, linef);
	}

	static void polygon_render(LuaStackValue args) {
		generic_render(args, polygonf);
	}

	static void line_render(LuaStackValue args) {
		generic_render(args, linef);
	}
	static void polygon_query(LuaStackValue args) {
		generic_query(args, polygonf);
	}
	static void line_query(LuaStackValue args) {
		generic_query(args, linef);
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

		submodule["polygon_apply"].bind_function(polygon_apply);
		submodule["polygon_render"].bind_function(polygon_render);
		submodule["polygon_query"].bind_function(polygon_query);
		submodule["line_apply"].bind_function(line_apply);
		submodule["line_render"].bind_function(line_render);
		submodule["line_query"].bind_function(line_query);
	}
}
