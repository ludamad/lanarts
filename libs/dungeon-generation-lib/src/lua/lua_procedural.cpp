/*
 * lua_procedural.cpp:
 *  Lua bindings for noise, cellular automata and random walks (map_procedural.h), registered on the
 *  map generation module (core.SourceMap):
 *
 *  noise_apply {map, seed, [area], [scale=16], [octaves=4], [persistence=0.5], [lacunarity=2],
 *               [falloff=0], bands = {{max = v, <selector fields>, <operator fields>}, ...}}
 *      -> squares changed. Each square takes the first band whose max is >= its noise value.
 *  noise_grid {seed, area, [scale], [octaves], [persistence], [lacunarity], [falloff]}
 *      -> rows of noise values, grid[y - area.y1 + 1][x - area.x1 + 1], roughly in [-1, 1].
 *  automaton_apply {map, rng, [area], [iterations=1], [radius=1], [outside_counts=false],
 *               rules = {{<selector fields>, neighbours = <selector>, [min_neighbours=0],
 *                         [max_neighbours], [chance=1], <operator fields>}, ...}}
 *      -> squares changed. Synchronous steps; each square takes the first rule that matches.
 *  random_walk_apply {map, rng, from_xy, steps, operator (may carry selector fields), [area], [passable = <selector>],
 *               [width=1], [toward_xy], [toward_bias=0.5]}
 *      -> the {x, y} the walk ended on.
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <lcommon/mtwist.h>
#include <lcommon/lua_geometry.h>

#include "Map.h"
#include "map_procedural.h"
#include "lua_ldungeon_impl.h"

namespace ldungeon_gen {

	static NoiseField noise_field_get(const LuaStackValue& args) {
		return NoiseField(args["seed"].to_int(),
				luawrap::defaulted(args["scale"], 16.0f),
				luawrap::defaulted(args["octaves"], 4),
				luawrap::defaulted(args["persistence"], 0.5f),
				luawrap::defaulted(args["lacunarity"], 2.0f));
	}

	static int lnoise_apply(lua_State* L) {
		LuaStackValue args(L, 1);
		MapPtr map = args["map"].as<MapPtr>();
		BBox area = luawrap::defaulted(args["area"], BBox(Pos(0, 0), map->size()));
		NoiseField field = noise_field_get(args);
		std::vector<NoiseBand> bands;
		LuaField lbands = args["bands"];
		for (int i = 1; i <= lbands.objlen(); i++) {
			LuaField band = lbands[i];
			bands.push_back({band["max"].to_num(), lua_conditional_operator_get(band)});
		}
		int applied = noise_apply(*map, area, field, luawrap::defaulted(args["falloff"], 0.0f), bands);
		lua_pushinteger(L, applied);
		return 1;
	}

	static int lnoise_grid(lua_State* L) {
		LuaStackValue args(L, 1);
		BBox area = args["area"].as<BBox>();
		NoiseField field = noise_field_get(args);
		float falloff = luawrap::defaulted(args["falloff"], 0.0f);
		lua_createtable(L, area.height(), 0);
		for (int y = area.y1; y < area.y2; y++) {
			lua_createtable(L, area.width(), 0);
			for (int x = area.x1; x < area.x2; x++) {
				lua_pushnumber(L, noise_falloff(field.at(x, y), area, x, y, falloff));
				lua_rawseti(L, -2, x - area.x1 + 1);
			}
			lua_rawseti(L, -2, y - area.y1 + 1);
		}
		return 1;
	}

	static int lautomaton_apply(lua_State* L) {
		LuaStackValue args(L, 1);
		MapPtr map = args["map"].as<MapPtr>();
		MTwist* rng = args["rng"].as<MTwist*>();
		BBox area = luawrap::defaulted(args["area"], BBox(Pos(0, 0), map->size()));
		std::vector<AutomatonRule> rules;
		LuaField lrules = args["rules"];
		for (int i = 1; i <= lrules.objlen(); i++) {
			LuaField rule = lrules[i];
			AutomatonRule r;
			r.selector = lua_selector_get(rule);
			r.neighbour = lua_selector_get(rule["neighbours"]);
			r.min_count = luawrap::defaulted(rule["min_neighbours"], 0);
			r.max_count = luawrap::defaulted(rule["max_neighbours"], 1 << 20);
			r.chance = luawrap::defaulted(rule["chance"], 1.0f);
			r.oper = lua_operator_get(rule);
			rules.push_back(r);
		}
		int iterations = luawrap::defaulted(args["iterations"], 1);
		int radius = luawrap::defaulted(args["radius"], 1);
		bool outside_counts = luawrap::defaulted(args["outside_counts"], false);
		int changed = 0;
		for (int i = 0; i < iterations; i++) {
			changed += automaton_step(*map, *rng, area, radius, outside_counts, rules);
		}
		lua_pushinteger(L, changed);
		return 1;
	}

	static int lrandom_walk_apply(lua_State* L) {
		LuaStackValue args(L, 1);
		MapPtr map = args["map"].as<MapPtr>();
		MTwist* rng = args["rng"].as<MTwist*>();
		BBox area = luawrap::defaulted(args["area"], BBox(Pos(0, 0), map->size()));
		RandomWalk walk;
		walk.from = args["from_xy"].as<Pos>();
		walk.steps = args["steps"].to_int();
		walk.oper = lua_conditional_operator_get(args["operator"]);
		walk.passable = args["passable"].isnil() ? Selector() : lua_selector_get(args["passable"]);
		walk.width = luawrap::defaulted(args["width"], 1);
		walk.has_target = !args["toward_xy"].isnil();
		walk.target = walk.has_target ? args["toward_xy"].as<Pos>() : Pos();
		walk.target_bias = luawrap::defaulted(args["toward_bias"], 0.5f);
		luawrap::push(L, random_walk_apply(*map, *rng, area, walk));
		return 1;
	}

	void lua_register_procedural(const LuaValue& submodule) {
		submodule["noise_apply"].bind_function(lnoise_apply);
		submodule["noise_grid"].bind_function(lnoise_grid);
		submodule["automaton_apply"].bind_function(lautomaton_apply);
		submodule["random_walk_apply"].bind_function(lrandom_walk_apply);
	}
}
