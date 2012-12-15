#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <common/unittest.h>

#include <net/lanarts_net.h>

#include "../../src/data/game_data.h"

#include <common/SerializeBuffer.h>

#include "../../src/stats/EquipmentStats.h"
#include "../../src/stats/Inventory.h"
#include "../../src/stats/combat_stats.h"

//static void assert_equal(DerivedStats& d1, DerivedStats& d2) {
//	UNIT_TEST_ASSERT(d1.damage == d2.damage);
//	UNIT_TEST_ASSERT(d1.power == d2.power);
//	UNIT_TEST_ASSERT(d1.reduction == d2.reduction);
//	UNIT_TEST_ASSERT(d1.resistance == d2.resistance);
//}
//static void assert_equal(ItemProperties& ip1, ItemProperties& ip2) {
//	UNIT_TEST_ASSERT(ip1.flags == ip2.flags);
//	UNIT_TEST_ASSERT(ip1.unknownness == ip2.unknownness);
//	assert_equal(ip1.magic, ip2.magic);
//	assert_equal(ip1.physical, ip2.physical);
//}
//
//template<typename T>
//static void assert_item_equal(T& t1, T& t2) {
//	UNIT_TEST_ASSERT(t1.id == t2.id);
//	assert_equal(t1.properties, t2.properties);
//}

static void assert_equal(Inventory& i1, Inventory& i2) {
	UNIT_TEST_ASSERT(i1.max_size() == i2.max_size());
	for (int i = 0; i < i1.max_size(); i++) {
		UNIT_TEST_ASSERT(i1.get(i) ==  i2.get(i));
	}
}

static void assert_equal(EquipmentStats& e1, EquipmentStats& e2) {
	UNIT_TEST_ASSERT(e1.weapon() == e2.weapon());
	UNIT_TEST_ASSERT(e1.armour() == e2.armour());
	UNIT_TEST_ASSERT(e1.projectile() ==  e2.projectile());
	assert_equal(e1.inventory, e2.inventory);
}

static void serialize_inventory() {
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

static void serialize_equipment() {
	const int TEST_N = 10;
	ItemEntry mock_item;
	//mock up some item data
	game_item_data.resize(TEST_N, &mock_item);

	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	EquipmentStats e1, e2;
//	e1.armour_slot() = Equipment(1);
//	e1.weapon_slot() = Weapon(1);
//	e1.projectile_slot() = Projectile(1);
	e1.inventory.add(Item(1));
	e1.serialize(serializer);

	e2.deserialize(serializer);

	assert_equal(e1, e2);

	game_item_data.clear();
}

static void serialize_combat_stats() {

}

void stat_serialize_tests() {
	UNIT_TEST(serialize_inventory);
	UNIT_TEST(serialize_equipment);
}
