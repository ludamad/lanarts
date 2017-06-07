/*
 * lua_map.cpp:
 *  Exposes the dungeon generation map to Lua
 */

#include <lcommon/strformat.h>
#include <vector>

#include <luawrap/luawrap.h>
#include <luawrap/members.h>
#include <luawrap/macros.h>

#include "Map.h"
#include "map_fill.h"
#include "tunnelgen.h"
#include "map_check.h"
#include "map_misc_ops.h"

#include "lua_ldungeon_impl.h"

namespace ldungeon_gen {

	/*****************************************************************************
	 *                          Square & flag helpers                            *
	 *****************************************************************************/
	static int flags_list(lua_State* L) {
		int flags;
		if (lua_isnumber(L, 1)) {
			flags = lua_tonumber(L, 1);
		} else {
			lua_pushfstring(L, "Expected integer in 'flags_list', got '%s'!",
					lua_typename(L, lua_type(L, 1)));
			return lua_error(L);
		}

		/* Push flag list */
		lua_newtable(L);
		LuaStackValue flag_list(L, -1);

		int iter = 1;
		while (iter <= flags) {
			if (iter & flags) {
				flag_list[flag_list.objlen() + 1] = iter;
			}
			iter <<= 1;
		}

		/* Return flag list */
		return 1;
	}

	static bool flags_match(int f1, int f2) {
		return (f1 & f2);
	}

	static int flags_combine(lua_State* L) {
		int nargs = lua_gettop(L);
		int flags = 0;
		if (lua_istable(L, 1)) {
			LuaStackValue table(L, 1);
			int objlen = table.objlen();
			for (int i = 1; i <= objlen; i++) {
				flags |= table[i].to_int();
			}
		} else {
			for (int i = 1; i <= nargs; i++) {
				flags |= lua_tointeger(L, i);
			}
		}

		lua_pushinteger(L, flags);
		return 1;

	}

	/* Handles either nil (==0), a list of flags (ORs them), or an integer */
	static int flags_get(LuaField flags) {
		lua_State* L = flags.luastate();

		flags.push();
		if (!lua_isnil(L, -1)) {
			lua_pushcfunction(L, flags_combine);
			lua_pushvalue(L, -2);
			lua_call(L, 1, 1);
			int flags = luawrap::get<int>(L, -1);
			lua_pop(L, 2);
			return flags;
		} else {
			lua_pop(L, 1);
			return 0;
		}
	}

	/* Parses a 'selector' from a table. All fields are optional. */
	Selector lua_selector_get(LuaField args) {
		bool has_content = args.has("matches_content");
		Selector selector(flags_get(args["matches_all"]),
				flags_get(args["matches_none"]),
				luawrap::defaulted(args["matches_content"], 0));
		selector.use_must_be_content = has_content;
		return selector;
	}

	Selector lua_selector_optional_get(LuaField args) {
		return args.isnil() ? Selector() : lua_selector_get(args);
	}

	/* Parses an 'operators' from a table. All fields are optional. */
	Operator lua_operator_get(LuaField args) {
		bool has_content = args.has("content");
		Operator oper(flags_get(args["add"]), flags_get(args["remove"]),
				flags_get(args["toggle"]),
				luawrap::defaulted(args["content"], 0));
                oper.group_value = luawrap::defaulted(args["group"], -1);
		return oper;
	}

	Operator lua_operator_optional_get(LuaField args) {
		return args.isnil() ? Operator() : lua_operator_get(args);
	}

	ConditionalOperator lua_conditional_operator_get(LuaField args) {
		return ConditionalOperator(lua_selector_get(args), lua_operator_get(args));
	}

	ConditionalOperator lua_conditional_operator_optional_get(
			LuaField args) {
		return args.isnil() ?
				ConditionalOperator(Selector(), Operator()) :
				lua_conditional_operator_get(args);
	}

	Square lua_square_get(LuaStackValue args) {
		using namespace luawrap;
		return Square(flags_get(args["flags"]),
				defaulted(args["content"], 0),
				defaulted(args["group"], ROOT_GROUP_ID));
	}

	void lua_square_push(lua_State* L, Square square) {
		lua_newtable(L);
		LuaStackValue table(L, -1);
		table["flags"] = square.flags;
		table["content"] = square.content;
		table["group"] = square.group;
	}

	/*****************************************************************************
	 *                          Map helpers                                      *
	 *****************************************************************************/

	/* Use a per-instance lua table as a fallback */
	static int lua_map_getter_fallback(lua_State* L) {
		LuaValue& variables = luawrap::get<MapPtr>(L, 1)->luafields;
		if (variables.empty()) {
			lua_pushnil(L);
		} else {
			variables.push();
			lua_pushvalue(L, 2);
			lua_gettable(L, -2);
		}
		return 1;
	}

	/* Use a per-instance lua table as a fallback */
	static int lua_map_setter_fallback(lua_State* L) {
		LuaValue& variables = luawrap::get<MapPtr>(L, 1)->luafields;
		if (variables.empty()) {
			variables.init(L);
			variables.newtable();
		}
		variables.push();
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_settable(L, -3);
		return 1;
	}

	static int lua_map_group_list(lua_State* L) {
		MapPtr map = luawrap::get<MapPtr>(L, 1);
		lua_newtable(L);
		LuaStackValue table(L, -1);
		// NOTE: creates 0-indexed group table
		for (int i = 0; i < map->groups.size(); i++) {
			table[i].newtable();
			table[i].push();
			LuaStackValue group(L, -1);
			group["id"] = map->groups[i].group_id;
			group["parent_id"] = map->groups[i].parent_group_id;
			group["children"] = map->groups[i].child_group_ids;
			group["area"] = map->groups[i].group_area;
			lua_pop(L, 1);
		}
		return 1;
	}

    static int lua_map_clear(lua_State* L) {
        MapPtr map = luawrap::get<MapPtr>(L, 1);
        map->clear();
        return 0;
    }
	LuaValue lua_mapmetatable(lua_State* L) {
		LUAWRAP_SET_TYPE(MapPtr&);

		LuaValue meta = luameta_new(L, "MapGen.Map");
		LuaValue methods = luameta_constants(meta);
		LuaValue getters = luameta_getters(meta);

		// Use table as fallback for getting values
		lua_pushcfunction(L, &lua_map_getter_fallback);
		luameta_defaultgetter(meta, LuaStackValue(L, -1));
		lua_pop(L, 1);
		// Use table as fallback for settings values
		lua_pushcfunction(L, &lua_map_setter_fallback);
		luameta_defaultsetter(meta, LuaStackValue(L, -1));
		lua_pop(L, 1);

		getters["groups"].bind_function(lua_map_group_list);

		methods["clear"].bind_function(lua_map_clear);

		LUAWRAP_GETTER(getters, size, OBJ->size());
		LUAWRAP_METHOD(methods, get, lua_square_push(L, (*OBJ)[luawrap::get<Pos>(L, 2)]));
		LUAWRAP_METHOD(methods, set, (*OBJ)[luawrap::get<Pos>(L, 2)] = lua_square_get(LuaStackValue(L, 3)));
		LUAWRAP_METHOD(methods, square_apply, (*OBJ)[luawrap::get<Pos>(L, 2)].apply(lua_operator_get(LuaStackValue(L, 3))));
		LUAWRAP_GETTER(methods, square_query, (*OBJ)[luawrap::get<Pos>(L, 2)].matches(lua_selector_get(LuaStackValue(L, 3))));

		return meta;
	}

	static std::string map_dump(MapPtr map) {
                std::string dump;
                for (int y = 0; y < map->height(); y++) { 
                    std::string line;
                    for (int x = 0; x < map->width(); x++) { 
                        int content = ((*map)[Pos(x,y)].content % 90);
                        line += char(' ' + content);
                    }
                    dump += line;
                    dump += '\n';
		}
                return dump;
	}

	static bool maps_equal(MapPtr map1, MapPtr map2) {
		using namespace luawrap;
                if (map1->size() != map2->size()) {
                    return false;
                }
                BBox area {{0,0}, map1->size()};
		FOR_EACH_BBOX(area, x, y) {
			if ((*map1)[Pos(x,y)].content != (*map2)[Pos(x,y)].content) {
				return false;
			}
		}
                return true;
	}


	static MapPtr map_create(LuaStackValue args) {
		using namespace luawrap;
		MapPtr ptr(
				new Map(defaulted(args["size"], Size()),
						lua_square_get(args)));
		ptr->luafields = args;
		return ptr;
	}

	/*****************************************************************************
	 *                      Area query helpers                                   *
	 *****************************************************************************/

	static const int QUERY_UPVALUE = 1;

	/* Helper for wrapping the object in a closure*/
	static int query_aux(lua_State* L) {
		using namespace luawrap;

		MapPtr map = get<MapPtr>(L, 1);
		group_t parent_group_id = get<int>(L, 2);
		BBox rect = get<BBox>(L, 3);

		AreaQueryPtr query = luawrap::get<AreaQueryPtr>(L,
				lua_upvalueindex(QUERY_UPVALUE));
		lua_pushboolean(L, query->matches(map, parent_group_id, rect));
		return 1;
	}

	LuaValue lua_querymetatable(lua_State* L) {
		LuaValue meta = luameta_new(L, "MapGen.AreaQuery");
		meta["__isareaquery"] = true;
		luameta_gc<AreaQueryPtr>(meta);
		return meta;
	}

	static LuaStackValue rectangle_query(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		RectangleQuery query(lua_selector_optional_get(args["fill_selector"]),
				defaulted(args["perimeter_width"], 0),
				lua_selector_optional_get(args["perimeter_selector"]));

		/* Create a closure which applies the function */
		luawrap::push(L, AreaQueryPtr(new RectangleQuery(query)));
		lua_pushcclosure(L, query_aux, 1);

		return LuaStackValue(L, -1);
	}


	struct LuaAreaQuery : public AreaQueryBase {
		LuaValue query;

		LuaAreaQuery(const LuaValue& oper) : query(oper) {
		}

		virtual bool matches(MapPtr map, group_t parent_group_id, const BBox& rect) {
			lua_State* L = query.luastate();

			/* Call lua function with passed arguments */
			query.push();
			return luawrap::call<bool>(L, map, parent_group_id, rect);
		}
	};

	/* Unpack an area query from a created closure */
	static AreaQueryPtr area_query_get(LuaField val) {
		lua_State* L = val.luastate();

		val.push();
		if (!lua_iscfunction(L, -1)) {
			return AreaQueryPtr(new LuaAreaQuery(LuaValue::pop_value(L)));
		}

		lua_getupvalue(L, -1, QUERY_UPVALUE);
		LuaStackValue upvalue(L, -1);
		bool is_query = upvalue.metatable()["__isareaquery"].to_bool();
		if (!is_query) {
			luawrap::error("area_operator_get: Not an area operator!");
		}

		AreaQueryPtr area_query = *(AreaQueryPtr*) upvalue.to_userdata();
		lua_pop(L, 2); /* pop closure and oper value */
		return area_query;
	}

	static AreaQueryPtr area_query_optional_get(LuaField val) {
		return val.isnil() ? AreaQueryPtr() : area_query_get(val);
	}

	static bool rectangle_query_apply(LuaStackValue args) {
		using namespace luawrap;
		LuaStackValue oper = rectangle_query(args);
		MapPtr map = args["map"].as<MapPtr>();
		bool applied = area_query_get(oper)->matches(
				map,
				defaulted(args["parent_group_id"], ROOT_GROUP_ID),
				defaulted(args["area"], BBox(Pos(), map->size()))
		);
		return applied;
	}

	/* Returns nil if none found, position otherwise */
	static int lfind_random_square(lua_State* L) {
		LuaStackValue args(L, 1);

		MapPtr map = args["map"].as<MapPtr>();
		BBox area = luawrap::defaulted(args["area"], BBox(Pos(0,0), map->size()));
		Pos xy;
		bool found = find_random_square(*args["rng"].as<MTwist*>(), map,
				area, lua_selector_get(args["selector"]), xy,
				luawrap::defaulted(args["max_attempts"],
						RANDOM_MATCH_MAX_ATTEMPTS));
		if (found) {
			if (!args["operator"].isnil()) {
				(*map)[xy].apply(lua_operator_get(args["operator"]));
			}
			luawrap::push(L, xy);
		} else {
			lua_pushnil(L);
		}
		return 1;
	}
	/* Retrieve data into a preallocated Lua table */
    static int lget_row_flags(lua_State* L) {
        MapPtr map = luawrap::get<MapPtr>(L, 1);
        // Note, Lua indexing corrections are applied
        int x1 = luaL_checkinteger(L, 3) - 1;
        int x2 = luaL_checkinteger(L, 4);
        int y = luaL_checkinteger(L, 5) - 1;
        // What to use outside of map bounds?
        double fill = luaL_checknumber(L, 6);
        int w = map->width(), h = map->height();
        Pos pos(x1, y);
        int i = 1;
        for (; pos.x < x2; pos.x++) {
            if (pos.y < 0 || pos.y >= h || pos.x < 0 || pos.x >= w) {
                lua_pushnumber(L, fill);
            } else {
                lua_pushnumber(L, (*map)[pos].flags);
            }
            lua_rawseti(L, 2, i);
            i++;
        }
        return 0;
    }
    /* Retrieve data into a preallocated Lua table */
    static int lget_row_content(lua_State* L) {
        MapPtr map = luawrap::get<MapPtr>(L, 1);
        // Note, Lua indexing corrections are applied
        int x1 = luaL_checkinteger(L, 3) - 1;
        int x2 = luaL_checkinteger(L, 4);
        int y = luaL_checkinteger(L, 5) - 1;
        // What to use outside of map bounds?
        int fill = lua_gettop(L) >= 6 ? luaL_checknumber(L, 6) : 0;
        int w = map->width(), h = map->height();
        Pos pos(x1, y);
        int i = 1;
        for (; pos.x < x2; pos.x++) {
            if (pos.y < 0 || pos.y >= h || pos.x < 0 || pos.x >= w) {
                lua_pushnumber(L, fill);
            } else {
                lua_pushnumber(L, (*map)[pos].content);
            }
            lua_rawseti(L, 2, i);
            i++;
        }
        return 0;
    }

	/*****************************************************************************
	 *                     Area operator helpers                                 *
	 *****************************************************************************/

	static const int OPER_UPVALUE = 1;

	/* Helper for wrapping the object in a closure*/
	int oper_aux(lua_State* L) {
		using namespace luawrap;

		MapPtr map = get<MapPtr>(L, 1);
		group_t parent_group_id = get<int>(L, 2);
		BBox rect = get<BBox>(L, 3);

		AreaOperatorPtr oper = luawrap::get<AreaOperatorPtr>(L,
				lua_upvalueindex(OPER_UPVALUE));
		lua_pushboolean(L, oper->apply(map, parent_group_id, rect));
		return 1;
	}

	LuaValue lua_opermetatable(lua_State* L) {
		LuaValue meta = luameta_new(L, "MapGen.AreaOperator");
		meta["__isareaoperator"] = true;
		luameta_gc<AreaOperatorPtr>(meta);
		return meta;
	}

	static LuaStackValue rectangle_operator(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		AreaQueryPtr query = area_query_optional_get(args["area_query"]);
		RectangleApplyOperator oper(query,
				lua_conditional_operator_optional_get(args["fill_operator"]),
				defaulted(args["perimeter_width"], 0),
				lua_conditional_operator_optional_get(args["perimeter_operator"]),
				defaulted(args["create_subgroup"], true));

		/* Create a closure which applies the function */
		luawrap::push(L, AreaOperatorPtr(new RectangleApplyOperator(oper)));
		lua_pushcclosure(L, oper_aux, 1);

		return LuaStackValue(L, -1);
	}

	static std::vector<Pos> rectangle_match(LuaStackValue args) {
		using namespace luawrap;
		LuaStackValue oper = rectangle_operator(args);
		Selector selector = lua_selector_get(args["selector"]);
		MapPtr map = args["map"].as<MapPtr>();
		BBox area = defaulted(args["area"], BBox(Pos(), map->size()));
		std::vector<Pos> matches;
		FOR_EACH_BBOX(area, x, y) {
			if ((*map)[Pos(x,y)].matches(selector)) {
				matches.push_back(Pos(x,y));
			}
		}
		return matches;
	}

	static bool rectangle_apply(LuaStackValue args) {
		using namespace luawrap;
		LuaStackValue oper = rectangle_operator(args);
		MapPtr map = args["map"].as<MapPtr>();
		bool applied = area_operator_get(oper)->apply(
				map,
				defaulted(args["parent_group_id"], ROOT_GROUP_ID),
				defaulted(args["area"], BBox(Pos(), map->size()))
		);
		return applied;
	}

	struct LuaAreaOperator : public AreaOperatorBase {
		LuaValue oper;

		LuaAreaOperator(const LuaValue& oper) : oper(oper) {
		}

		virtual bool apply(MapPtr map, group_t parent_group_id, const BBox& rect) {
			lua_State* L = oper.luastate();

			/* Call lua function with passed arguments */
			int nargs = lua_gettop(L);
			oper.push();
			luawrap::push(L, map);
			luawrap::push(L, parent_group_id);
			luawrap::push(L, rect);
			lua_call(L, 3, LUA_MULTRET);
			int ret = lua_gettop(L) - nargs;

			// Assume success if no return, less error-prone
			bool success = (ret <= 0 || lua_toboolean(L, nargs + 1));
			lua_pop(L, ret); // pop return values

			return success;
		}
	};

	AreaOperatorPtr area_operator_get(LuaField val) {
		lua_State* L = val.luastate();

		val.push();
		if (!lua_iscfunction(L, -1)) {
			return AreaOperatorPtr(new LuaAreaOperator(LuaValue::pop_value(L)));
		}

		lua_getupvalue(L, -1, OPER_UPVALUE);
		LuaStackValue upvalue(L, -1);
		bool is_oper = upvalue.metatable()["__isareaoperator"].to_bool();
		if (!is_oper) {
			luawrap::error("area_operator_get: Not a valid operator!");
		}

		AreaOperatorPtr area_oper = *(AreaOperatorPtr*) upvalue.to_userdata();
		lua_pop(L, 2); /* pop closure and oper value */
		return area_oper;
	}


    static bool larea_fully_connected(LuaStackValue args) {
        using namespace luawrap;
        MapPtr map = args["map"].as<MapPtr>();
        Selector unfilled = lua_selector_get(args["unfilled_selector"]);
        Operator mark = lua_operator_get(args["mark_operator"]);
        Selector marked = lua_selector_get(args["marked_selector"]);
        BBox area = defaulted(args["area"], BBox(Pos(), map->size()));
        return area_fully_connected(*map, area, unfilled, mark, marked);
    }

    static void lerode_diagonal_pairs(LuaStackValue args) {
        using namespace luawrap;
        lua_State* L = args.luastate();

        MapPtr map = args["map"].as<MapPtr>();
        //Get RNG setup for map generation
        MTwist* mtwist = args["rng"].as<MTwist*>();

        Selector selector = lua_selector_get(args["selector"]);
        BBox area = luawrap::defaulted(args["area"], BBox(Pos(), map->size()));
        erode_diagonal_pairs(*map, *mtwist, area, selector);
    }

	static LuaStackValue tunnel_operator(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		//Get RNG setup for map generation
		MTwist* mtwist = args["rng"].as<MTwist*>();

		LuaValue vs = args["validity_selector"];
		Selector vfill_selector =
				vs.isnil() ?
						Selector() : lua_selector_optional_get(vs["fill_selector"]);
		Selector vperimeter_selector =
				vs.isnil() ?
						Selector() :
						lua_selector_optional_get(vs["perimeter_selector"]);

		LuaValue cs = args["completion_selector"];
		Selector cfill_selector =
				cs.isnil() ?
						Selector() : lua_selector_optional_get(cs["fill_selector"]);
		Selector cperimeter_selector =
				cs.isnil() ?
						Selector() :
						lua_selector_optional_get(cs["perimeter_selector"]);

		ConditionalOperator fill_oper = lua_conditional_operator_get(
				args["fill_operator"]);
		ConditionalOperator perimeter_oper = lua_conditional_operator_get(
				args["perimeter_operator"]);

		int perimeter_width = defaulted(args["perimeter_width"], 0);
		Range size_range = args["size_range"].as<Range>();
		Range tunnels_per_room_range =
				args["tunnels_per_room_range"].as<Range>();

		TunnelCheckSettings tunnel_selector(vfill_selector, vperimeter_selector,
				cfill_selector, cperimeter_selector);

		TunnelGenOperator oper(*mtwist, tunnel_selector, fill_oper,
				perimeter_oper, perimeter_width, size_range,
				tunnels_per_room_range);
		/* Create a closure which applies the function */
		luawrap::push(L, AreaOperatorPtr(new TunnelGenOperator(oper)));
		lua_pushcclosure(L, oper_aux, 1);

		return LuaStackValue(L, -1);
	}

    static void __submap_apply(LuaStackValue args) {
        MapPtr map = args["map"].as<MapPtr>();
        MapPtr submap = args["submap"].as<MapPtr>();
        Pos xy(args["x"].to_int(), args["y"].to_int());
        BBox area = luawrap::defaulted(args["submap_area"], BBox(Pos(), submap->size()));
        Selector selector = lua_selector_get(args["selector"]);
        Selector submap_selector = lua_selector_get(args["submap_selector"]);
        submap_apply(map, submap, xy, area, selector, submap_selector);
    }

    static void __perimeter_apply(LuaStackValue args) {
        MapPtr map = args["map"].as<MapPtr>();
        BBox area = luawrap::defaulted(args["area"], BBox(Pos(), map->size()));
        ConditionalOperator oper = lua_conditional_operator_get(args["operator"]);
        Selector candidate = lua_selector_get(args["candidate_selector"]);
        Selector criteria = lua_selector_get(args["inner_selector"]);
        perimeter_apply(map, area, candidate, criteria, oper);
    }

	static bool tunnel_apply(LuaStackValue args) {
		using namespace luawrap;
		LuaStackValue oper = tunnel_operator(args);
		bool applied = area_operator_get(oper)->apply(
				args["map"].as<MapPtr>(),
				defaulted(args["parent_group_id"], ROOT_GROUP_ID),
				args["area"].as<BBox>()
		);
		return applied;
	}

	/* Binding that simply provides random placement in an area, leaves overlap check to child area operator. */
	static LuaStackValue random_placement_operator(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		//Get RNG setup for map generation
        MTwist* mtwist = args["rng"].as<MTwist*>();
		bool create_subgroup = defaulted(args["create_subgroup"], true);

		Range size_range = args["size_range"].as<Range>();
		Range amount_of_placements_range =
				args["amount_of_placements_range"].as<Range>();

		RandomPlacementApplyOperator oper(*mtwist,
				amount_of_placements_range, size_range,
				area_operator_get(args["child_operator"]), create_subgroup);

		/* Create a closure which applies the function */
		luawrap::push(L,
				AreaOperatorPtr(new RandomPlacementApplyOperator(oper)));
		lua_pushcclosure(L, oper_aux, 1);

		return LuaStackValue(L, -1);
	}

	static bool random_placement_apply(LuaStackValue args) {
		using namespace luawrap;
		LuaStackValue oper = random_placement_operator(args);
		MapPtr map =  args["map"].as<MapPtr>();
		bool applied = area_operator_get(oper)->apply(
				map,
				defaulted(args["parent_group_id"], ROOT_GROUP_ID),
				defaulted(args["area"], BBox(Pos(), map->size()))
		);
		return applied;
	}

	/*****************************************************************************
	 *                          Register bindings                                *
	 *****************************************************************************/

	static void lua_register_placement_functions(const LuaValue& submodule) {
		submodule["map_create"].bind_function(map_create);
		submodule["rectangle_operator"].bind_function(rectangle_operator);
		submodule["tunnel_operator"].bind_function(tunnel_operator);
		submodule["rectangle_apply"].bind_function(rectangle_apply);
		submodule["tunnel_apply"].bind_function(tunnel_apply);
		submodule["submap_apply"].bind_function(__submap_apply);
		submodule["perimeter_apply"].bind_function(__perimeter_apply);

		submodule["random_placement_operator"].bind_function(
				random_placement_operator);

		submodule["random_placement_apply"].bind_function(
				random_placement_apply);

		submodule["rectangle_query"].bind_function(rectangle_query_apply);
		submodule["rectangle_match"].bind_function(rectangle_match);
		submodule["rectangle_criteria"].bind_function(rectangle_query);
		submodule["find_random_square"].bind_function(lfind_random_square);
        submodule["get_row_flags"].bind_function(lget_row_flags);
        submodule["get_row_content"].bind_function(lget_row_content);
        submodule["erode_diagonal_pairs"].bind_function(lerode_diagonal_pairs);
        submodule["area_fully_connected"].bind_function(larea_fully_connected);

		LUAWRAP_SET_TYPE(LuaStackValue);
		LUAWRAP_GETTER(submodule, random_place,
				random_place(OBJ["area"].as<BBox>(), *OBJ["rng"].as<MTwist*>(), luawrap::defaulted(OBJ["size"], Size())));
	}

	void lua_register_map(const LuaValue& submodule) {
		lua_State* L = submodule.luastate();

		luawrap::install_userdata_type<MapPtr, lua_mapmetatable>();
		luawrap::install_userdata_type<AreaOperatorPtr, lua_opermetatable>();
		luawrap::install_userdata_type<AreaQueryPtr, lua_querymetatable>();

		lua_register_bsp(submodule);
		lua_register_placement_functions(submodule);

		submodule["ROOT_GROUP"] = ROOT_GROUP_ID;

		submodule["FLAG_SOLID"] = FLAG_SOLID;
		submodule["FLAG_TUNNEL"] = FLAG_TUNNEL;
		submodule["FLAG_HAS_OBJECT"] = FLAG_HAS_OBJECT;
		submodule["FLAG_NEAR_PORTAL"] = FLAG_NEAR_PORTAL;
		submodule["FLAG_PERIMETER"] = FLAG_PERIMETER;
        submodule["FLAG_SEETHROUGH"] = FLAG_SEETHROUGH;
        submodule["FLAG_RESERVED1"] = FLAG_RESERVED1;
        submodule["FLAG_RESERVED2"] = FLAG_RESERVED2;
		submodule["FLAGS_ALL"] = (int)-1;
		for (int i = 1; i <= 8; i++) {
			std::string str = format("FLAG_CUSTOM%d", i);
			submodule[str] = FLAG_RESERVED2 << i;
		}

		submodule["maps_equal"].bind_function(maps_equal);
		submodule["map_dump"].bind_function(map_dump);
		submodule["flags_list"].bind_function(flags_list);
		submodule["flags_match"].bind_function(flags_match);
		submodule["flags_combine"].bind_function(flags_combine);
	}
}
