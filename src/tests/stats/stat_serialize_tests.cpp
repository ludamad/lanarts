#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <net/lanarts_net.h>

#include "../tests.h"

#include "../../serialize/SerializeBuffer.h"

#include "../../stats/Inventory.h"
#include "../../stats/Equipment.h"
#include "../../stats/combat_stats.h"
#include "../../data/game_data.h"

static void assert_equal(DerivedStats& d1, DerivedStats& d2) {
	UNIT_TEST_ASSERT(d1.damage == d2.damage);
	UNIT_TEST_ASSERT(d1.power == d2.power);
	UNIT_TEST_ASSERT(d1.reduction == d2.reduction);
	UNIT_TEST_ASSERT(d1.resistance == d2.resistance);
}
static void assert_equal(ItemProperties& ip1, ItemProperties& ip2) {
	UNIT_TEST_ASSERT(ip1.flags == ip2.flags);
	UNIT_TEST_ASSERT(ip1.unknownness == ip2.unknownness);
	assert_equal(ip1.magic, ip2.magic);
	assert_equal(ip1.physical, ip2.physical);
}

template<typename T>
static void assert_item_equal(T& t1, T& t2) {
	UNIT_TEST_ASSERT(t1.id == t2.id);
	assert_equal(t1.properties, t2.properties);
}

static void assert_equal(Inventory& i1, Inventory& i2) {
	UNIT_TEST_ASSERT(i1.max_size() == i2.max_size());
	for (int i = 0; i < i1.max_size(); i++) {
		assert_item_equal(i1.get(i).item, i2.get(i).item);
		UNIT_TEST_ASSERT(i1.get(i).amount == i2.get(i).amount);
	}
}

static void assert_equal(Equipment& e1, Equipment& e2) {
	assert_item_equal(e1.weapon, e2.weapon);
	assert_item_equal(e1.armour, e2.armour);
	assert_item_equal(e1.projectile, e2.projectile);
	UNIT_TEST_ASSERT(e1.projectile_amnt == e2.projectile_amnt);
	assert_equal(e1.inventory, e2.inventory);
}

static void serialize_inventory() {
	const int TEST_N = 10;
	//mock up some item data
	game_item_data.resize(TEST_N, ItemEntry("", "", "", 0, 0, "", "", false));

	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	Inventory inv1, inv2;
	for (int i = 0; i < TEST_N; i++) {
		inv1.add(Item(i), 1);
	}

	inv1.serialize(serializer);
	inv2.deserialize(serializer);

	assert_equal(inv1, inv2);

	game_item_data.clear();
}

static void serialize_equipment() {
	const int TEST_N = 10;
	//mock up some item data
	game_item_data.resize(TEST_N, ItemEntry("", "", "", 0, 0, "", "", false));
	game_weapon_data.resize(TEST_N);
	game_armour_data.resize(TEST_N);

	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	Equipment e1, e2;
	e1.armour = Armour(1);
	e1.weapon = Weapon(1);
	e1.projectile = Projectile(1);
	e1.inventory.add(Item(1), 1);
	e1.serialize(serializer);

	e2.deserialize(serializer);

	assert_equal(e1, e2);

	game_item_data.clear();
	game_weapon_data.clear();
	game_armour_data.clear();
}

static void serialize_combat_stats() {

}

void stat_serialize_tests() {
	UNIT_TEST(serialize_inventory);
	UNIT_TEST(serialize_equipment);
}
