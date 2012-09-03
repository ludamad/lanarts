/*
 * items.cpp:
 *  Define item states. These are defined in terms of a base item, and applied properties.
 */

#include "../data/game_data.h"
#include "../serialize/SerializeBuffer.h"

#include "items.h"

_ItemEntry& _Projectile::item_entry() const {
	const std::string& name = projectile_entry().name;
	return _game_item_data.at(_get_item_by_name(name.c_str()));
}

_Item _Projectile::as_item() const {
	const std::string& name = projectile_entry().name;
	return _Item(_get_item_by_name(name.c_str()), properties);
}

_ProjectileEntry& _Projectile::projectile_entry() const {
	return game_projectile_data.at(id);
}

_ItemEntry& _Weapon::item_entry() const {
	const std::string& name = weapon_entry().name;
	return _game_item_data.at(_get_item_by_name(name.c_str()));
}

_Item _Weapon::as_item() const {
	const std::string& name = weapon_entry().name;
	return _Item(_get_item_by_name(name.c_str()), properties);
}

_WeaponEntry& _Weapon::weapon_entry() const {
	return game_weapon_data.at(id);
}

_ItemEntry& _Item::item_entry() const {
	LANARTS_ASSERT(id > -1);
	return _game_item_data.at(id);
}

_Armour _Item::as_armour() const {
	return _Armour(item_entry().equipment_id, properties);
}

_ItemEntry& _Armour::item_entry() const {
	const std::string& name = armour_entry().name;
	return _game_item_data.at(_get_item_by_name(name.c_str()));
}

_ArmourEntry& _Armour::armour_entry() const {
	return game_armour_data.at(id);
}

_Item _Armour::as_item() const {
	const std::string& name = armour_entry().name;
	return _Item(_get_item_by_name(name.c_str()), properties);
}

_Projectile _Item::as_projectile() const {
	return _Projectile(item_entry().equipment_id, properties);
}

_Weapon _Item::as_weapon() const {
	return _Weapon(item_entry().equipment_id, properties);
}

_ProjectileEntry& _Item::projectile_entry() const {
	LANARTS_ASSERT(is_projectile());
	return game_projectile_data.at(item_entry().equipment_id);
}

_WeaponEntry& _Item::weapon_entry() const {
	LANARTS_ASSERT(is_weapon());
	return game_weapon_data.at(item_entry().equipment_id);
}

bool _Item::is_normal_item() const {
	return item_entry().equipment_type == _ItemEntry::NONE;
}

bool _Item::is_armour() const {
	return item_entry().equipment_type == _ItemEntry::ARMOUR;
}

bool _Item::is_projectile() const {
	return item_entry().equipment_type == _ItemEntry::PROJECTILE;
}

bool _Item::is_weapon() const {
	return item_entry().equipment_type == _ItemEntry::WEAPON;
}

bool _ItemProperties::operator ==(const _ItemProperties& properties) const {
	return magic == properties.magic && physical == properties.physical
			&& flags == properties.flags;
}

bool _Projectile::operator ==(const _Projectile& projectile) const {
	return id == projectile.id && properties == projectile.properties;
}

bool _Weapon::operator ==(const _Weapon& weapon) const {
	return id == weapon.id && properties == weapon.properties;
}

bool _Item::operator ==(const _Item& item) const {
	return id == item.id && properties == item.properties;
}

void _ItemProperties::serialize(SerializeBuffer& serializer) {
	serializer.write(magic);
	serializer.write(physical);
	serializer.write_int(flags);
}

void _ItemProperties::deserialize(SerializeBuffer& serializer) {
	serializer.read(magic);
	serializer.read(physical);
	serializer.read_int(flags);
}

void _Armour::serialize(SerializeBuffer& serializer) {
	serializer.write(id);
	properties.serialize(serializer);
}

void _Armour::deserialize(SerializeBuffer& serializer) {
	serializer.read(id);
	properties.deserialize(serializer);
}

void _Projectile::serialize(SerializeBuffer& serializer) {
	serializer.write(id);
	properties.serialize(serializer);
}

void _Projectile::deserialize(SerializeBuffer& serializer) {
	serializer.read(id);
	properties.deserialize(serializer);
}

void _Weapon::serialize(SerializeBuffer& serializer) {
	serializer.write(id);
	properties.serialize(serializer);
}

void _Weapon::deserialize(SerializeBuffer& serializer) {
	serializer.read(id);
	properties.deserialize(serializer);
}

void _Item::serialize(SerializeBuffer& serializer) {
	serializer.write(id);
	properties.serialize(serializer);
}

void _Item::deserialize(SerializeBuffer& serializer) {
	serializer.read(id);
	properties.deserialize(serializer);
}

