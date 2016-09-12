/*
 * lua_bsp.cpp:
 *  Bindings & wrappings for navigating a BSP tree in Lua
 */

#include <lcommon/strformat.h>
#include <lcommon/math_util.h>

#include <luawrap/luawrap.h>
#include <luawrap/members.h>
#include <luawrap/macros.h>

#include "../bsp.hpp"

#include "Map.h"
#include "map_fill.h"
#include "tunnelgen.h"
#include "map_check.h"

#include "lua_ldungeon_impl.h"

namespace ldungeon_gen {

	static LuaStackValue bsp_operator(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		BSPApplyOperator oper(*args["rng"].as<MTwist*>(),
				area_operator_get(args["child_operator"]),
				defaulted(args["minimum_node_size"], Size()),
				defaulted(args["randomize_size"], true),
				defaulted(args["split_depth"], 8),
				defaulted(args["create_subgroup"], true));

		/* Create a closure which applies the function */
		luawrap::push(L, AreaOperatorPtr(new BSPApplyOperator(oper)));
		lua_pushcclosure(L, oper_aux, 1);

		return LuaStackValue(L, -1);
	}

	struct LuaTreeReconstructor : public ITCODBspCallback {
		LuaTreeReconstructor(LuaStackValue cache) : cache(cache) {
			this->L = cache.luastate();
		}

		virtual bool visitNode(TCODBsp* node, void * __unused) {
			lua_pushlightuserdata(L, node);
			lua_newtable(L);
			LuaStackValue lnode(L, -1);
			lnode["area"] = BBox(node->x, node->y, node->x + node->w, node->y + node->h);
			lua_rawset(L, cache.index());
			return true;
		}

		void get_luanode(LuaField field, TCODBsp* node) {
			lua_pushlightuserdata(L, node);
			lua_rawget(L, cache.index());
			field.pop();
		}

		void push_final_tree(TCODBsp* root) {
			lua_pushnil(L);
			while (lua_next(L, cache.index()) != 0) {
				TCODBsp* bsp = (TCODBsp*)lua_touserdata(L, -2);
				LuaStackValue lnode(L, -1);
				get_luanode(lnode["left"], bsp->getLeft());
				get_luanode(lnode["right"], bsp->getRight());
				get_luanode(lnode["parent"], bsp->getFather());
				lnode["was_hsplit"] = bsp->horizontal;
				lnode["was_vsplit"] = !bsp->horizontal;
				lnode["depth"] = bsp->level;

				lua_pop(L, 1);
			}
			lua_pushlightuserdata(L, root);
			lua_rawget(L, cache.index());
		}

		lua_State* L;
		LuaStackValue cache;
	};

	static LuaStackValue bsp_split(LuaStackValue args) {
		using namespace luawrap;
		lua_State* L = args.luastate();

		MapPtr map = args["map"].as<MapPtr>();
		BBox rect = defaulted(args["area"], BBox(Pos(), map->size()));
		Size node_size = defaulted(args["minimum_node_size"], Size());
		int split_depth = defaulted(args["split_depth"], 1);
		float max_hratio = defaulted(args["max_width_ratio"], 1.5f);
		float max_vratio = defaulted(args["max_height_ratio"], 1.5f);

		TCODBsp* bsp = new TCODBsp(rect.x1, rect.y1, rect.width(), rect.height());

		bsp->splitRecursive(*args["rng"].as<MTwist*>(), split_depth, node_size.w, node_size.h, max_hratio, max_vratio);

		// create the dungeon from the bsp
		lua_newtable(L); // reconstruction table
		LuaTreeReconstructor reconstructor(LuaStackValue(L, -1));
		bsp->traverseInvertedLevelOrder(&reconstructor, NULL);
		reconstructor.push_final_tree(bsp);

		delete bsp;

		return LuaStackValue(L, -1);
	}

	// Constants for traversal closures.
	// Important strings are stored in upvalues for efficient access
	static const int LEFT_IDX = lua_upvalueindex(1), RIGHT_IDX = lua_upvalueindex(2);

	typedef void (*traversal_f)(lua_State* L);

	static void lbsp_pushchild(lua_State* L, int fieldidx) {
		lua_pushvalue(L, fieldidx);
		lua_rawget(L, -2);
	}
	// Only for lbsp_iterate!
	static void lbsp_visit(lua_State* L) {
		lua_pushvalue(L, 1); // Access first argument of bsp_iterate, the closure to use to visit
		lua_call(L, 0, 0);
	}

	static void lbsp_inorder(lua_State* L) {
		if (!lua_isnil(L, -1)) {
			lbsp_pushchild(L, LEFT_IDX);
			lbsp_inorder(L);

			lbsp_visit(L);

			lbsp_pushchild(L, RIGHT_IDX);
			lbsp_inorder(L);
		}
		lua_pop(L, 1);
	}
	static void lbsp_preorder(lua_State* L) {
		if (!lua_isnil(L, -1)) {
			lbsp_visit(L);

			lbsp_pushchild(L, LEFT_IDX);
			lbsp_inorder(L);

			lbsp_pushchild(L, RIGHT_IDX);
			lbsp_inorder(L);
		}
		lua_pop(L, 1);
	}
	static void lbsp_postorder(lua_State* L) {
		if (!lua_isnil(L, -1)) {
			lbsp_pushchild(L, LEFT_IDX);
			lbsp_inorder(L);

			lbsp_pushchild(L, RIGHT_IDX);
			lbsp_inorder(L);

			lbsp_visit(L);
		}
		lua_pop(L, 1);
	}

	static traversal_f funcs[] = {&lbsp_inorder, &lbsp_preorder, &lbsp_postorder};
	static int lbsp_iterate(lua_State* L) {
		int nargs = lua_gettop(L);
		traversal_f tfunc = &lbsp_inorder;
		if (lua_isnumber(L, 1)) {
			int id = squish(lua_tointeger(L, 1), 0, 2);
			tfunc = funcs[id];
			lua_remove(L, 1);
		}
		(*tfunc)(L);
		return 0;
	}

	void lua_register_bsp(const LuaValue& submodule) {
		lua_State* L = submodule.luastate();

		submodule["bsp_operator"].bind_function(bsp_operator);
		submodule["bsp_split"].bind_function(bsp_split);
		submodule["BSP_INORDER"] = 0;
		submodule["BSP_PREORDER"] = 1;
		submodule["BSP_POSTORDER"] = 2;
		lua_pushstring(L, "left");
		lua_pushstring(L, "right");
		lua_pushcclosure(L, lbsp_iterate, 2);
		submodule["bsp_apply"].pop();
	}
}
