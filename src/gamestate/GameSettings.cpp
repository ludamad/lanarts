/*
 * Contains configuration for lanarts
 * All data relevant to chosen game mode, class, configuration options etc is stored here.
 */

#include "GameSettings.h"

#include <lcommon/SerializeBuffer.h>

static void write_or_assert_hash(SerializeBuffer& sb, unsigned int hash,
        bool is_writing) {
    if (is_writing) {
        sb.write_int(hash);
    } else {
        unsigned int val;
        sb.read(val);
        if (val != hash) {
            printf("Values 0x%X and 0x%X do not match!\n", val, hash);
            LANARTS_ASSERT(false);
        }
    }
}

void GameSettings::serialize_gameplay_settings(SerializeBuffer& serializer) const {
	serializer.write(regen_on_death);
	serializer.write(class_type);
	serializer.write(conntype);
        write_or_assert_hash(serializer, 0xBABEBABE, true);
}

void GameSettings::deserialize_gameplay_settings(SerializeBuffer& serializer) {
	serializer.read(regen_on_death);
	serializer.read(class_type);
	serializer.read(conntype);
        write_or_assert_hash(serializer, 0xBABEBABE, false);
}
