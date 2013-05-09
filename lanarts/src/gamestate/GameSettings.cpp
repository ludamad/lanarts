/*
 * Contains configuration for lanarts
 * All data relevant to chosen game mode, class, configuration options etc is stored here.
 */

#include "GameSettings.h"

#include <lcommon/SerializeBuffer.h>

void GameSettings::serialize_gameplay_settings(SerializeBuffer& serializer) const {
	serializer.write(regen_on_death);
	serializer.write(class_type);
	serializer.write(conntype);
}

void GameSettings::deserialize_gameplay_settings(SerializeBuffer& serializer) {
	serializer.read(regen_on_death);
	serializer.read(class_type);
	serializer.read(conntype);
}
