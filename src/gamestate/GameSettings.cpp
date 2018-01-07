/*
 * Contains configuration for lanarts
 * All data relevant to chosen game mode, class, configuration options etc is stored here.
 */

#include "GameSettings.h"

#include <lcommon/SerializeBuffer.h>
#include <luawrap/luawrap.h>

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

template <typename T>
static void optional_fill(const LuaValue& lsettings, const char* key, T& value) {
    if (!lsettings[key].isnil()) {
        value = lsettings[key].as<T>();
    }
}

void GameSettings::parse(LuaValue& lsettings) {
    optional_fill(lsettings, "font", font);
    optional_fill(lsettings, "menu_font", menu_font);
    optional_fill(lsettings, "fullscreen", fullscreen);
    optional_fill(lsettings, "regen_level_on_death", regen_on_death);
    optional_fill(lsettings, "view_width", view_width);
    optional_fill(lsettings, "view_height", view_height);
    optional_fill(lsettings, "ip", ip);
    optional_fill(lsettings, "port", port);
    optional_fill(lsettings, "lobby_server_url", lobby_server_url);
    optional_fill(lsettings, "steps_per_draw", steps_per_draw);
    if (steps_per_draw < 1)
        steps_per_draw = 1;
    optional_fill(lsettings, "frame_action_repeat",
                 frame_action_repeat);
    optional_fill(lsettings, "free_memory_while_idle",
                 free_memory_while_idle);
    if (frame_action_repeat < 0)
        frame_action_repeat = 0;
    optional_fill(lsettings, "invincible", invincible);
    optional_fill(lsettings, "time_per_step", time_per_step);
    optional_fill(lsettings, "draw_diagnostics", draw_diagnostics);
    optional_fill(lsettings, "username", username);
    optional_fill(lsettings, "network_debug_mode",
                 network_debug_mode);
    optional_fill(lsettings, "savereplay_file", savereplay_file);
    optional_fill(lsettings, "loadreplay_file", loadreplay_file);
    optional_fill(lsettings, "verbose_output", verbose_output);
    optional_fill(lsettings, "autouse_health_potions",
                 autouse_health_potions);
    optional_fill(lsettings, "autouse_mana_potions",
                 autouse_mana_potions);
    optional_fill(lsettings, "keep_event_log", keep_event_log);
    optional_fill(lsettings, "comparison_event_log",
                 comparison_event_log);

    if (lsettings["connection_type"].isnil()) {
        std::string connname = lsettings["connection_type"].to_str();
        if (connname == "none") {
            conntype = GameSettings::NONE;
        } else if (connname == "client") {
            conntype = GameSettings::CLIENT;
        } else if (connname == "server") {
            conntype = GameSettings::SERVER;
        }
    }

    optional_fill(lsettings, "class", class_type);
}
