/*
 * lua_map.cpp:
 *  Exposes the dungeon generation map to Lua
 */

#include <luawrap/luawrap.h>
#include <luawrap/members.h>
#include <luawrap/macros.h>

#include "Map.h"
#include "map_fill.h"
#include "tunnelgen.h"
#include "map_check.h"

namespace ldungeon_gen {

	/*****************************************************************************
	 *                          Square & flag helpers                            *
	 *****************************************************************************/
	static int flags_list(lua_State* L) {
		int flags;
		if (lua_isnumber(L, 1)) {
			flags = lua_tonumber(L, 1);
		} else {
			Square sqr = luawrap::get<Square>(L, 1);
			flags = sqr.flags;
		}

		/* Push flag list */
		lua_newtable(L);
		LuaStackValue flag_list(L,-1);

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
	static Selector selector_get(LuaField args) {
		bool has_content = args.has("matches_content");
		Selector selector(flags_get(args["matches_all"]),
				flags_get(args["matches_none"]),
				luawrap::defaulted(args["matches_content"], 0));
		selector.use_must_be_content = has_content;
		return selector;
	}

	/* Parses an 'operators' from a table. All fields are optional. */
	static Operator operator_get(LuaField args) {
		bool has_content = args.has("content");
		Operator oper(flags_get(args["add"]),
				flags_get(args["remove"]),
				flags_get(args["toggle"]),
				luawrap::defaulted(args["content"], 0));
		oper.use_content_value = has_content;
		return oper;
	}

	static ConditionalOperator conditional_operator_get(LuaField args) {
		return ConditionalOperator(selector_get(args), operator_get(args));
	}

	static ConditionalOperator conditional_operator_optional_get(LuaField args) {
		return args.isnil() ? ConditionalOperator(Selector(), Operator()) : conditional_operator_get(args);
	}

	static Square square_create(LuaStackValue args) {
		using namespace luawrap;
		return Square(defaulted(args["flags"], 0),
				defaulted(args["content"], 0),
				defaulted(args["group"], ROOT_GROUP_ID));
	}

	LuaValue lua_squaremetatable(lua_State* L) {
		LuaValue meta = luameta_new(L, "MapGen.Square");
		LuaValue methods = luameta_constants(meta);

		LuaValue getters = luameta_getters(meta);
		luawrap::bind_getter(getters["flags"], &Square::flags);
		luawrap::bind_getter(getters["content"], &Square::content);
		luawrap::bind_getter(getters["group"], &Square::group);

		return meta;
	}

	/*****************************************************************************
	 *                          Map helpers                                      *
	 *****************************************************************************/
	LuaValue lua_mapmetatable(lua_State* L) {
		LUAWRAP_SET_TYPE(MapPtr&);

		LuaValue meta = luameta_new(L, "MapGen.Map");
		LuaValue methods = luameta_constants(meta);
		LuaValue getters = luameta_getters(meta);

		LUAWRAP_GETTER(getters, size, OBJ->size());
		LUAWRAP_GETTER(methods, get, (*OBJ)[luawrap::get<Pos>(L, 2)]);
		LUAWRAP_METHOD(methods, set, (*OBJ)[luawrap::get<Pos>(L, 2)] = luawrap::get<Square>(L, 3));

		return meta;
	}

	static MapPtr map_create(LuaStackValue args) {
		using namespace luawrap;
		MapPtr ptr(
				new Map(defaulted(args["size"], Size()),
						defaulted(args["fill_value"], Square())));
		return ptr;
	}


	/*****************************************************************************
	 *                   Area operator helpers                                   *
	 *****************************************************************************/

	const int OPER_UPVALUE = luawrap::UPVALUES_USED + 1;

	/* Helper for wrapping the object in a closure*/
	static void oper_aux(LuaStackValue map, group_t parent_group_id, const BBox& rect) {
		lua_State* L = map.luastate();
		AreaOperatorPtr oper = luawrap::get<AreaOperatorPtr>(L, lua_upvalueindex(OPER_UPVALUE));
		oper->apply(map.as<MapPtr>(), parent_group_id, rect);
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

		RectangleApplyOperator oper(
				conditional_operator_optional_get(args["fill_operator"]),
				defaulted(args["perimeter_width"], 0),
				conditional_operator_optional_get(args["perimeter_operator"]),
				defaulted(args["create_subgroup"], true));

		/* Create a closure which applies the function */
		luawrap::push_function(L, "MapGen.rectangle_operator instance", oper_aux);
		luawrap::push(L, AreaOperatorPtr(new RectangleApplyOperator(oper)));
		lua_setupvalue(L, -2, OPER_UPVALUE);

		return LuaStackValue(L, -1);
	}

//	static LuaAreaOperator : public AreaOperatorBase {
//		LuaValue oper;
//
//		LuaAreaOperator(const LuaValue& oper) : oper(oper) {
//
//		}
//
//		virtual bool apply(Map& map, group_t parent_group_id, const BBox& rect) {
//			oper.push();
//			luawrap::call<void>(L, )
//		}
//	};
//
	static LuaStackValue bsp_operator(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		//Get RNG setup for map generation
		luawrap::registry(L)["MapGenRNG"].push();
		MTwist* mtwist = (MTwist*)lua_touserdata(L, -1);

		args["child_operator"].push();
		lua_getupvalue(L, -1, OPER_UPVALUE);
		LuaStackValue upvalue(L, -1);
		bool is_oper = upvalue.metatable()["__isareaoperator"].to_bool();
		if (!is_oper) {
			luawrap::error("bsp_operator: Not a valid operator!");
		}
		AreaOperatorPtr area_oper = *(AreaOperatorPtr*)lua_touserdata(L, -1);
		lua_pop(L, 2); /* pop operator and upvalue */

		BSPApplyOperator oper(*mtwist, area_oper,
				defaulted(args["minimum_node_size"], Size()),
				defaulted(args["randomize_size"], true),
				defaulted(args["split_depth"], 8),
				defaulted(args["create_subgroup"], true));

		/* Create a closure which applies the function */
		luawrap::push_function(L, "MapGen.bsp_operator instance", oper_aux);
		luawrap::push(L, AreaOperatorPtr(new BSPApplyOperator(oper)));
		lua_setupvalue(L, -2, OPER_UPVALUE);

		return LuaStackValue(L, -1);
	}

	static LuaStackValue tunnel_operator(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		//Get RNG setup for map generation
		luawrap::registry(L)["MapGenRNG"].push();
		MTwist* mtwist = (MTwist*)lua_touserdata(L, -1);
		
		LuaValue vs = args["validity_selector"];
 		Selector vfill_selector = vs.isnil() ? Selector() : selector_get(args["fill_selector"]);
 		Selector vperimeter_selector = vs.isnil() ? Selector() : selector_get(args["perimeter_selector"]);

		LuaValue cs = args["completion_selector"];
 		Selector cfill_selector = cs.isnil() ? Selector() : selector_get(args["fill_selector"]);
 		Selector cperimeter_selector = cs.isnil() ? Selector() : selector_get(args["perimeter_selector"]);

		ConditionalOperator fill_oper = conditional_operator_get(args["fill_operator"]);
		ConditionalOperator perimeter_oper = conditional_operator_get(args["perimeter_operator"]);

		int padding = defaulted(args["padding"], 1);
		Range size_range = args["size_range"].as<Range>();
		Range tunnels_per_room_range  = args["tunnels_per_room_range "].as<Range>();

	        TunnelSelector tunnel_selector(vfill_selector, vperimeter_selector,
                                  cfill_selector, cperimeter_selector);

                TunnelGenOperator oper(*mtwist, tunnel_selector, fill_oper,
                                perimeter_oper, padding, size_range, tunnels_per_room_range);	
		/* Create a closure which applies the function */
		luawrap::push_function(L, "MapGen.tunnel_operator instance", oper_aux);
		luawrap::push(L, AreaOperatorPtr(new TunnelGenOperator(oper)));
		lua_setupvalue(L, -2, OPER_UPVALUE);

		return LuaStackValue(L, -1);
	}
	/*****************************************************************************
	 *                          Register bindings                                *
	 *****************************************************************************/
	void lua_register_map(const LuaValue& module, MTwist* mtwist) {
		lua_State* L = module.luastate();

		luawrap::install_userdata_type<Square, lua_squaremetatable>();
		luawrap::install_userdata_type<MapPtr, lua_mapmetatable>();
		luawrap::install_userdata_type<AreaOperatorPtr, lua_opermetatable>();

		/* TODO: Find a better way to deal with this static variable hack. */
		lua_pushlightuserdata(L, (void*)mtwist);
		luawrap::registry(L)["MapGenRNG"].pop();

		module["ROOT_GROUP"] = ROOT_GROUP_ID;

		module["FLAG_SOLID"] = FLAG_SOLID;
		module["FLAG_TUNNEL"] = FLAG_TUNNEL;
		module["FLAG_HAS_OBJECT"] = FLAG_HAS_OBJECT;
		module["FLAG_NEAR_PORTAL"] = FLAG_NEAR_PORTAL;
		module["FLAG_PERIMETER"] = FLAG_PERIMETER;

		module["flags_list"].bind_function(flags_list);
		module["flags_match"].bind_function(flags_match);
		module["flags_combine"].bind_function(flags_combine);

		module["square"].bind_function(square_create);

		module["map_create"].bind_function(map_create);
		module["rectangle_operator"].bind_function(rectangle_operator);
		module["bsp_operator"].bind_function(bsp_operator);
		module["tunnel_operator"].bind_function(tunnel_operator);
	}
}
