#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <lcommon/unittest.h>

#include <net/lanarts_net.h>

#include <lcommon/SerializeBuffer.h>

#include "data/game_data.h"

#include "stats/EquipmentStats.h"
#include "stats/Inventory.h"
#include "stats/combat_stats.h"

SUITE(stat_serialize_tests) {

	static void assert_equal(Inventory& i1, Inventory& i2) {
		CHECK(i1.max_size() == i2.max_size());
		for (int i = 0; i < i1.max_size(); i++) {
			CHECK(i1.get(i) == i2.get(i));
		}
	}

	static void assert_equal(EquipmentStats& e1, EquipmentStats& e2) {
		CHECK(e1.weapon() == e2.weapon());
		CHECK(e1.armour() == e2.armour());
		CHECK(e1.projectile() == e2.projectile());
		assert_equal(e1.inventory, e2.inventory);
	}

	TEST(serialize_inventory) {
		const int TEST_N = 10;
		ItemEntry mock_item;
		//mock up some item data
		game_item_data.resize(TEST_N, &mock_item);

		SerializeBuffer serializer = SerializeBuffer::plain_buffer();
		Inventory inv1, inv2;
		for (int i = 0; i < TEST_N; i++) {
			inv1.add(Item(i));
		}

		inv1.serialize(serializer);
		inv2.deserialize(serializer);

		assert_equal(inv1, inv2);

		game_item_data.clear();
	}

	TEST(serialize_equipment) {
		const int TEST_N = 10;
		ItemEntry mock_item;
		//mock up some item data
		game_item_data.resize(TEST_N, &mock_item);

		SerializeBuffer serializer = SerializeBuffer::plain_buffer();
		EquipmentStats e1, e2;
		e1.inventory.add(Item(1));
		e1.serialize(serializer);

		e2.deserialize(serializer);

		assert_equal(e1, e2);

		game_item_data.clear();
	}
}
