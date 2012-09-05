/*
 * items.h:
 *  Define item state. These are defined in terms of a base item, and applied properties.
 */

#include <cstring>

#include "../../serialize/SerializeBuffer.h"

#include "EquipmentEntry.h"
#include "ItemEntry.h"
#include "ProjectileEntry.h"
#include "WeaponEntry.h"

#include "items.h"

bool ItemProperties::operator ==(const ItemProperties& properties) const {
	if (memcmp(&properties.cooldown_modifiers, &this->cooldown_modifiers,
			sizeof(CooldownModifiers) != 0)) {
		return false;
	}
	if (memcmp(&properties.stat_modifiers, &this->stat_modifiers,
			sizeof(StatModifiers) != 0)) {
		return false;
	}
	if (memcmp(&properties.damage, &this->damage,
			sizeof(DamageStats) != 0)) {
		return false;
	}
	if (properties.effect_modifiers.status_effects
			!= effect_modifiers.status_effects) {
		return false;
	}
	if (properties.flags != flags || properties.unknownness != unknownness) {
		return false;
	}
	return true;
}

void ItemProperties::serialize(SerializeBuffer& serializer) {
	serializer.write(this->cooldown_modifiers);
	serializer.write(this->stat_modifiers);
	serializer.write(this->damage);
	serializer.write_int(this->flags);
	serializer.write(this->unknownness);

	serializer.write(this->unknownness);
	serializer.write_container(this->effect_modifiers.status_effects);
}

void ItemProperties::deserialize(SerializeBuffer& serializer) {
	serializer.read(this->cooldown_modifiers);
	serializer.read(this->stat_modifiers);
	serializer.read(this->damage);
	serializer.read_int(this->flags);
	serializer.read(this->unknownness);
	serializer.read_container(this->effect_modifiers.status_effects);
}

ItemEntry& Item::item_entry() const {
	return *game_item_data.at(id);
}

EquipmentEntry& Item::equipment_entry() const {
	return dynamic_cast<EquipmentEntry&>(item_entry());
}

ProjectileEntry& Item::projectile_entry() const {
	return dynamic_cast<ProjectileEntry&>(item_entry());
}

WeaponEntry& Item::weapon_entry() const {
	return dynamic_cast<WeaponEntry&>(item_entry());
}

bool Item::is_normalItem() const {
	ItemEntry* item_entry = game_item_data.at(id);
	return (dynamic_cast<EquipmentEntry*>(item_entry) == NULL);
}

bool Item::is_equipment() const {
	ItemEntry* item_entry = game_item_data.at(id);
	return (dynamic_cast<EquipmentEntry*>(item_entry) != NULL);
}

bool Item::is_projectile() const {
	ItemEntry* item_entry = game_item_data.at(id);
	return (dynamic_cast<ProjectileEntry*>(item_entry) != NULL);
}

bool Item::is_weapon() const {
	ItemEntry* item_entry = game_item_data.at(id);
	return (dynamic_cast<WeaponEntry*>(item_entry) != NULL);
}

bool Item::is_same_item(const Item & item) const {
	return id == item.id && properties == item.properties;
}

void Item::serialize(SerializeBuffer & serializer) {
	serializer.write_int(id);
	serializer.write_int(amount);
	properties.serialize(serializer);
}

void Item::deserialize(SerializeBuffer & serializer) {
	serializer.read_int(id);
	serializer.read_int(amount);
	properties.deserialize(serializer);
}

