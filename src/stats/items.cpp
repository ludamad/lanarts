///*
// * items.cpp:
// *  Define item states. These are defined in terms of a base item, and applied properties.
// */
//
//#include "../data/game_data.h"
//#include "../serialize/SerializeBuffer.h"
//
//#include "items.h"
//
//ItemEntry& Item::item_entry() const {
//	const std::string& name = projectile_entry().name;
//	return _gameItem_data.at(getItem_by_name(name.c_str()));
//}
//
//Item Item::asItem() const {
//	const std::string& name = projectile_entry().name;
//	return Item(getItem_by_name(name.c_str()), properties);
//}
//
//ProjectileEntry& Item::projectile_entry() const {
//	return get_projectile_entry(id);
//}
//
//ItemEntry& _Weapon::item_entry() const {
//	const std::string& name = weapon_entry().name;
//	return _gameItem_data.at(getItem_by_name(name.c_str()));
//}
//
//Item _Weapon::asItem() const {
//	const std::string& name = weapon_entry().name;
//	return Item(getItem_by_name(name.c_str()), properties);
//}
//
//WeaponEntry& _Weapon::weapon_entry() const {
//	return get_weapon_entry(id);
//}
//
//ItemEntry& Item::item_entry() const {
//	LANARTS_ASSERT(id > -1);
//	return _gameItem_data.at(id);
//}
//
//EquipmentItem Item::as_armour() const {
//	return EquipmentItem(item_entry().equipment_id, properties);
//}
//
//ItemEntry& EquipmentItem::item_entry() const {
//	const std::string& name = armour_entry().name;
//	return _gameItem_data.at(getItem_by_name(name.c_str()));
//}
//
//EquipmentEntry& EquipmentItem::armour_entry() const {
//	return get_equipment_entry(id);
//}
//
//Item EquipmentItem::asItem() const {
//	const std::string& name = armour_entry().name;
//	return Item(getItem_by_name(name.c_str()), properties);
//}
//
//Item Item::as_projectile() const {
//	return Item(item_entry().equipment_id, properties);
//}
//
//_Weapon Item::as_weapon() const {
//	return _Weapon(item_entry().equipment_id, properties);
//}
//
//ProjectileEntry& Item::projectile_entry() const {
//	LANARTS_ASSERT(is_projectile());
//	return get_projectile_entry(item_entry().equipment_id);
//}
//
//WeaponEntry& Item::weapon_entry() const {
//	LANARTS_ASSERT(is_weapon());
//	return get_weapon_entry(item_entry().equipment_id);
//}
//
//bool Item::is_normalItem() const {
//	return item_entry().equipment_type == EquipmentEntry::NONE;
//}
//
//bool Item::is_armour() const {
//	return item_entry().equipment_type == EquipmentEntry::ARMOUR;
//}
//
//bool Item::is_projectile() const {
//	return item_entry().equipment_type == EquipmentEntry::PROJECTILE;
//}
//
//bool Item::is_weapon() const {
//	return item_entry().equipment_type == EquipmentEntry::WEAPON;
//}
//
//bool ItemProperties::operator ==(const ItemProperties& properties) const {
//	return magic == properties.magic && physical == properties.physical
//			&& flags == properties.flags;
//}
//
//bool Item::operator ==(const Item& projectile) const {
//	return id == projectile.id && properties == projectile.properties;
//}
//
//bool _Weapon::operator ==(const Item& weapon) const {
//	return id == weapon.id && properties == weapon.properties;
//}
//
//bool Item::same_item(const Item& item) const {
//	return id == item.id && properties == item.properties;
//}
//
//void ItemProperties::serialize(SerializeBuffer& serializer) {
//	serializer.write(magic);
//	serializer.write(physical);
//	serializer.write_int(flags);
//}
//
//void ItemProperties::deserialize(SerializeBuffer& serializer) {
//	serializer.read(magic);
//	serializer.read(physical);
//	serializer.read_int(flags);
//}
//
//void EquipmentItem::serialize(SerializeBuffer& serializer) {
//	serializer.write(id);
//	properties.serialize(serializer);
//}
//
//void EquipmentItem::deserialize(SerializeBuffer& serializer) {
//	serializer.read(id);
//	properties.deserialize(serializer);
//}
//
//void Item::serialize(SerializeBuffer& serializer) {
//	serializer.write(id);
//	properties.serialize(serializer);
//}
//
//void Item::deserialize(SerializeBuffer& serializer) {
//	serializer.read(id);
//	properties.deserialize(serializer);
//}
//
//void _Weapon::serialize(SerializeBuffer& serializer) {
//	serializer.write(id);
//	properties.serialize(serializer);
//}
//
//void _Weapon::deserialize(SerializeBuffer& serializer) {
//	serializer.read(id);
//	properties.deserialize(serializer);
//}
//
//void Item::serialize(SerializeBuffer& serializer) {
//	serializer.write(id);
//	properties.serialize(serializer);
//}
//
//void Item::deserialize(SerializeBuffer& serializer) {
//	serializer.read(id);
//	properties.deserialize(serializer);
//}
//
