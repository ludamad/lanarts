#include <algorithm>
#include <cstdio>
#include <cstring>

#include <lcommon/unittest.h>
#include <lcommon/mtwist.h>

#include "Map.h"
#include "map_fill.h"
#include "tunnelgen.h"

using namespace ldungeon_gen;

TEST(square_operator_apply) {
	const int OFF_VALUE = 1 << 0;
	const int ON_VALUE = 1 << 1;
	const int FLIP_VALUE = 1 << 2;
	const int TEST_CONTENT = 5;
	Square square(OFF_VALUE, 0);
	Operator oper(ON_VALUE, OFF_VALUE, FLIP_VALUE, TEST_CONTENT);
	square.apply(oper);
	CHECK(square.content == TEST_CONTENT);
	CHECK(square.flags == (ON_VALUE | FLIP_VALUE));
}

TEST(square_selector_matches) {
	const int IS_OFF = 1 << 0;
	const int IS_ON = 1 << 1;
	const int TEST_CONTENT = 5;
	Square square(IS_ON, TEST_CONTENT);
	/* Expected to match */
	{
		Selector selector(0, 0);
		CHECK(square.matches(selector));
	}
	{
		Selector selector(0, 0, TEST_CONTENT);
		CHECK(square.matches(selector));
	}
	{
		Selector selector(IS_ON, 0, TEST_CONTENT);
		CHECK(square.matches(selector));
	}
	{
		Selector selector(0, IS_OFF);
		CHECK(square.matches(selector));
	}
	/* Expected to not match */
	{
		Selector selector(IS_OFF, 0, TEST_CONTENT);
		CHECK(!square.matches(selector));
	}
	{
		Selector selector(0, 0, 0);
		CHECK(!square.matches(selector));
	}
	{
		Selector selector(0, IS_ON);
		CHECK(!square.matches(selector));
	}
}

TEST(map_rectangle_fill) {
	const uint16 TEST_CONTENT = 5;
	const uint16 TEST_GROUP = 10;

	Map m(Size(10, 10), Square(FLAG_SOLID));
	Selector selector(FLAG_SOLID, 0);
	Operator oper(FLAG_HAS_OBJECT /*turn on*/, FLAG_SOLID /*turn off*/, 0,
			TEST_CONTENT, TEST_GROUP);
	const BBox bounds(0, 0, 10, 10);
	rectangle_apply(m, bounds, ConditionalOperator(selector, oper));
	FOR_EACH_BBOX(bounds, x, y)
	{
		CHECK(
				m[Pos(x, y)].matches(
						Selector(FLAG_HAS_OBJECT, FLAG_SOLID, TEST_CONTENT,
								TEST_GROUP)));
	}
}

static const Selector SELECT_ALL(0, 0);
static const Size MAP_SIZE(80, 80);

TEST(bsp_rect_apply) {
	MTwist randomizer;
	Map map(MAP_SIZE);

	/* 0, 1, 2 are used as map values*/
	ConditionalOperator fill_oper(SELECT_ALL, Operator(0, FLAG_SOLID /* remove solid flag */, 0, 1));
	ConditionalOperator perimeter_oper(SELECT_ALL, Operator(FLAG_PERIMETER /* add perimeter flag */, 0, 0, 2));
	/* Operator to carve out each room */
	RectangleOperator rect_oper(fill_oper, 1, perimeter_oper);
	bsp_rect_apply(map, BSPSettings(Size(10, 10), true, 8), randomizer,
			BBox(Pos(0, 0), MAP_SIZE), rect_oper);

	TunnelGenSettings tgs(1, Range(1,1), Range(1,1));
	ConditionalOperator tunnel_oper(SELECT_ALL, Operator(0, 0, 0, 3));
	ConditionalOperator tunnel_perimeter_oper(SELECT_ALL, Operator(FLAG_PERIMETER, 0, 0, 4));
	generate_tunnels(map, tgs, randomizer, tunnel_oper, tunnel_perimeter_oper);

	// render the level
	for (int y = 0; y < MAP_SIZE.h; y++) {
		for (int x = 0; x < MAP_SIZE.w; x++) {
			int n = map[Pos(x,y)].content;
			int g = map[Pos(x,y)].group;
			bool solid = map[Pos(x,y)].matches_flags(FLAG_SOLID);
			if (n == 0) {
				printf( solid ? "# ": "0 ");
			} else if (n == 1) {
				printf("%c ", char('a'+(g%24)));
			} else if (n == 2) {
				printf(". ");
			} else if (n == 3) {
				printf("+ ");
			} else if (n == 4) {
				printf("= ");
			}
		}
		printf("\n");
	}
}
