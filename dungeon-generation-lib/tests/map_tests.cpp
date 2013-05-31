#include <algorithm>
#include <cstdio>
#include <cstring>

#include <lcommon/unittest.h>
#include <lcommon/mtwist.h>
#include <lcommon/SerializeBuffer.h>

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

	Map map(Size(10, 10), Square(FLAG_SOLID));

	Selector selector(FLAG_SOLID, 0);
	Operator oper(FLAG_HAS_OBJECT /*turn on*/, FLAG_SOLID /*turn off*/, 0,
			TEST_CONTENT);
	RectangleApplyOperator rect_oper(ConditionalOperator(selector, oper));

	const BBox bounds(Pos(0, 0), map.size());
	rect_oper.apply(map, ROOT_GROUP_ID, bounds);

	FOR_EACH_BBOX(bounds, x, y)
	{
		CHECK(
				map[Pos(x, y)].matches(
						Selector(FLAG_HAS_OBJECT, FLAG_SOLID, TEST_CONTENT)));
	}
}

static bool serialized_correctly(const Map& map) {
	SerializeBuffer buffer = SerializeBuffer::plain_buffer();
	map.serialize(buffer);
	Map newmap;
	newmap.deserialize(buffer);
	return map == newmap;
}

static const Selector SELECT_ALL(0, 0);
static const Size MAP_SIZE(80, 40);

TEST(test_map_generation) {
	MTwist randomizer;
	Map map(MAP_SIZE);

	/* 0, 1, 2 are used as map content values*/
	/* Create rooms */{
		ConditionalOperator fill_oper(SELECT_ALL,
				Operator(0, FLAG_SOLID /* remove solid flag */, 0, 1));
		ConditionalOperator perimeter_oper(SELECT_ALL,
				Operator(FLAG_PERIMETER /* add perimeter flag */, 0, 0, 2));
		/* Operator to carve out each room */
		RectangleApplyOperator rect_oper(fill_oper, 1, perimeter_oper);
		BSPApplyOperator bsp_oper(randomizer, rect_oper, Size(8,8), true, 6);
		bsp_oper.apply(map, ROOT_GROUP_ID, BBox(Pos(0, 0), MAP_SIZE));
	}

	/* Create tunnels */{
		ConditionalOperator fill_oper(SELECT_ALL,
				Operator(FLAG_TUNNEL, FLAG_SOLID, 0, 3));
		ConditionalOperator perimeter_oper(SELECT_ALL,
				Operator(FLAG_SOLID | FLAG_TUNNEL | FLAG_PERIMETER, 0, 0, 4));
		Selector is_invalid(FLAG_TUNNEL, FLAG_SOLID);
		Selector is_finished(0, FLAG_SOLID | FLAG_TUNNEL);
		TunnelGenOperator tunnel_oper(randomizer, is_invalid, is_finished,
				fill_oper, perimeter_oper, 1, Range(1, 2), Range(1, 1));
		tunnel_oper.apply(map, ROOT_GROUP_ID, BBox(Pos(0, 0), MAP_SIZE));
	}

	// render the level
	for (int y = 0; y < MAP_SIZE.h; y++) {
		for (int x = 0; x < MAP_SIZE.w; x++) {
			int n = map[Pos(x, y)].content;
			int g = map[Pos(x, y)].group;
			bool solid = map[Pos(x, y)].matches_flags(FLAG_SOLID);
			if (n == 0) {
				printf(solid ? "# " : "0 ");
			} else if (n == 1) {
//				printf("%c ", char('a' + (g % 24)));
				printf("  ");
			} else if (n == 2) {
//				printf(". ");
				printf("# ");
			} else if (n == 3) {
//				printf("+ ");
				printf("  ");
			} else if (n == 4) {
//				printf("= ");
				printf("# ");
			}
		}
		printf("\n");
	}

	CHECK(serialized_correctly(map));
}
