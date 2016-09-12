/*
 * lua_tunnelgen.cpp:
 *  Wrapper for tunnel generation code.
 */

#include <luawrap/luawrap.h>
#include <luawrap/members.h>
#include <luawrap/macros.h>

#include "lua_ldungeon.h"
#include "tunnelgen.h"

#include "lua_ldungeon_impl.h"

namespace ldungeon_gen {
	static TunnelCheckSettings tunnelcheck_get(LuaStackValue args) {
		using namespace luawrap;

		LuaField vs = args["validity_selector"], cs =
				args["completion_selector"];
		Selector vfill_selector, vperimeter_selector;
		Selector cfill_selector, cperimeter_selector;
		if (!vs.isnil()) {
			vfill_selector = lua_selector_optional_get(vs["fill_selector"]);
			vperimeter_selector = lua_selector_optional_get(
					vs["perimeter_selector"]);
		}
		if (!cs.isnil()) {
			cfill_selector = lua_selector_optional_get(cs["fill_selector"]);
			cperimeter_selector = lua_selector_optional_get(
					cs["perimeter_selector"]);
		}

		return TunnelCheckSettings(vfill_selector, vperimeter_selector,
				cfill_selector, cperimeter_selector,
				defaulted(args["avoid_group"], -1),
				defaulted(args["end_group"], -1));
	}

	static TunnelFillSettings tunnelfill_get(LuaStackValue args) {
		using namespace luawrap;

		ConditionalOperator fill_oper = lua_conditional_operator_get(
				args["fill_operator"]);
		ConditionalOperator perimeter_oper = lua_conditional_operator_get(
				args["perimeter_operator"]);

		return TunnelFillSettings(fill_oper,
				defaulted(args["perimeter_width"], 0), perimeter_oper,
				defaulted(args["width"], 1), defaulted(args["max_length"], 0),
				defaulted(args["turn_chance"], 0.00f));
	}

	static bool lua_tunnel_generate(LuaStackValue args) {
		lua_State* L = args.luastate();

		return tunnel_generate(args["map"].as<MapPtr>(), *args["rng"].as<MTwist*>(),
				args["top_left_xy"].as<Pos>(), args["direction"].as<Pos>(),
				tunnelcheck_get(args), tunnelfill_get(args));
	}

	void lua_register_tunnelgen(const LuaValue& module) {
		module["tunnel_generate"].bind_function(lua_tunnel_generate);
		module["LEFT"] = Pos(-1, 0);
		module["RIGHT"] = Pos(+1, 0);
		module["DOWN"] = Pos(0, +1);
		module["UP"] = Pos(0, -1);
	}
}
