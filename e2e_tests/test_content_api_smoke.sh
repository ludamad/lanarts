#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

grep -q "_lanarts_content_package_matrix_json" CMakeLists.txt
grep -q "_lanarts_content_map_matrix_json" CMakeLists.txt
grep -q "_lanarts_content_fight_setup_json" CMakeLists.txt
grep -q "_lanarts_content_build_matrix_json" CMakeLists.txt
grep -q "_lanarts_content_micro_drills_json" CMakeLists.txt
grep -q "_lanarts_content_action_guide_json" CMakeLists.txt
grep -q "_lanarts_content_presentation_json" CMakeLists.txt
grep -q "buildArchetypes" web/content_lab/shell.html
grep -q "microDrills" web/content_lab/shell.html
grep -q "actionGuide" web/content_lab/shell.html
grep -q "microDrill" web/content_lab/shell.html
grep -q "Drill Scorecard" web/content_lab/shell.html
grep -q "lanarts_content_presentation_json" web/content_lab/shell.html
grep -q "Sweeps only" web/content_lab/shell.html
grep -q "avg_package_sweep_active_count" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_micro_score" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_micro_thread_cues" runtime/engine/StartBalanceRun.lua
grep -q "MICRO x" runtime/objects/ContentLabHazards.lua
grep -q "THREAD OPEN" runtime/objects/ContentLabHazards.lua
grep -q "avg_package_micro_counter_windows" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_micro_punish_total" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_micro_punish_break_total" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_pressure_sync_bursts" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_pressure_sync_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_pressure_unique_components" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_pressure_dominance_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_aggro_ally_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_aggro_inferred_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_aggro_switches" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_rolls_total" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_animation_frames" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_buildup_fx" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_face_swaps" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_lock_cues" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_surge_cues" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_finale_fx" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_suspense_pips" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_roll_guard_cues" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_anti_repeat_rerolls" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_cooldown_trim_frames" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_high_roll_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_distinct_faces" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_max_face_share" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_longest_high_streak" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_longest_utility_streak" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_high_streak_breaks" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_low_streak_breaks" runtime/engine/StartBalanceRun.lua
grep -q "dice-animation" runtime/engine/ContentApi.lua
grep -q "dice-motion" runtime/engine/ContentApi.lua
grep -q "dice-surge" runtime/engine/ContentApi.lua
grep -q "dice-finale" runtime/engine/ContentApi.lua
grep -q "dice-face-spread" runtime/engine/ContentApi.lua
grep -q "dice-high-share" runtime/engine/ContentApi.lua
grep -q "dice-face-cap" runtime/engine/ContentApi.lua
grep -q "dice-high-streak-cap" runtime/engine/ContentApi.lua
grep -q "dice-dry-streak-cap" runtime/engine/ContentApi.lua
grep -q "avg_package_rescue_avg_clutch_margin" runtime/engine/ContentApi.lua
grep -q "sync-beats" runtime/engine/ContentApi.lua
grep -q "beat-variety" runtime/engine/ContentApi.lua
grep -q "Beat mix" runtime/engine/ContentApi.lua
grep -q "skill-buffer" runtime/engine/ContentApi.lua
grep -q "DICE_MINOR_COOLDOWN_TRIM" runtime/spells/Effects.moon
grep -q "DICE_FULL_BAG_MASK" runtime/spells/Effects.moon
grep -q "DICE_ROLL_LOCK_STAGE_FRAMES" runtime/spells/Effects.moon
grep -q "DICE_ROLL_SURGE_STAGE_FRAMES" runtime/spells/Effects.moon
grep -q "dicebinder_emit_roll_fx" runtime/spells/Effects.moon
grep -q "dicebinder_emit_roll_tick_fx" runtime/spells/Effects.moon
grep -q "dicebinder_update_roll_surge" runtime/spells/Effects.moon
grep -q "draw_die_face" runtime/ui/ProceduralEffects.lua
grep -q "trail_roll" runtime/ui/ProceduralEffects.lua
grep -q "surge_stage" runtime/ui/ProceduralEffects.lua
grep -q "safe_xy_for_map" runtime/ui/ImpactFX.lua
grep -q "fx_bounds" runtime/maps/NewMaps.moon
grep -q "\"MARGIN\"" runtime/objects/ContentLabHazards.lua
grep -q "\"SYNC\"" runtime/objects/ContentLabHazards.lua
grep -q "\"MIX\"" runtime/objects/ContentLabHazards.lua
grep -q "orbiting die ghosts" web/content_lab/README.md
grep -q "avg_package_rescue_early_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_timing_score" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_avg_clutch_margin" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_avg_timed_pull_margin" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_window_cues" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_late_cover_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_late_commit_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_perfect_commit_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_avg_commit_distance" runtime/engine/StartBalanceRun.lua
grep -q "rescue-commit" runtime/engine/ContentApi.lua
grep -q "CUT PERFECT" runtime/objects/ContentLabHazards.lua
grep -q "avg_package_rescue_counter_windows" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_fatal_misses" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_timed_pull_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_pull_discipline_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_taunt_strain_rate" runtime/engine/StartBalanceRun.lua
grep -q "taunt-strain" runtime/engine/ContentApi.lua
grep -q "TAUNT STRAIN" runtime/objects/ContentLabHazards.lua
grep -q "pull-discipline" runtime/engine/ContentApi.lua
grep -q "avg_package_rescue_pull_snap_bursts" runtime/engine/StartBalanceRun.lua
grep -q "PERFECT SNAP" runtime/objects/ContentLabHazards.lua
grep -q "avg_package_rescue_snap_ready_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_broad_pull_index" runtime/engine/StartBalanceRun.lua
grep -q "package_rescue_active_snap_ready" runtime/engine/StartBalanceRun.lua
grep -q "broad-pull" runtime/engine/ContentApi.lua
grep -q "avg_package_rescue_snap_ready_rate" runtime/engine/ContentApi.lua
grep -q "\"SNAP\"" runtime/objects/ContentLabHazards.lua
grep -q "mana-sapper-ritual-package" runtime/engine/ContentCatalog.lua
grep -q "mana-sapper-ritual-package" web/content_lab/shell.html
grep -q "hell-warrior-flame-package" runtime/engine/ContentCatalog.lua
grep -q "hell-warrior-flame-package" web/content_lab/shell.html
grep -q "stara-pain-star-package" runtime/engine/ContentCatalog.lua
grep -q "stara-pain-star-package" web/content_lab/shell.html
grep -q "pixulloch-web-package" runtime/engine/ContentCatalog.lua
grep -q "pixulloch-web-package" web/content_lab/shell.html
grep -q "waxanarian-venom-package" runtime/engine/ContentCatalog.lua
grep -q "waxanarian-venom-package" web/content_lab/shell.html
grep -q "nesso-arrow-storm-package" runtime/engine/ContentCatalog.lua
grep -q "nesso-arrow-storm-package" web/content_lab/shell.html
grep -q "crossing bone scythes" runtime/engine/ContentCatalog.lua
grep -q "crossing bone scythes" web/content_lab/shell.html
grep -q "DREAD on mechanic hits" runtime/engine/ContentCatalog.lua
grep -q "DREAD on mechanic hits" web/content_lab/shell.html
grep -q "package.micro_punish.label" runtime/objects/ContentLabHazards.lua
grep -q "source or player" runtime/objects/ContentLabHazards.lua
grep -q "MICRO" runtime/objects/ContentLabHazards.lua
grep -q "BEAT" runtime/objects/ContentLabHazards.lua
grep -q "draw_package_beat_wheel" runtime/objects/ContentLabHazards.lua
grep -q "draw_micro_streak_marker" runtime/objects/ContentLabHazards.lua
grep -q "compact beat wheel" web/content_lab/README.md
grep -q "micro-streak halo" web/content_lab/README.md
grep -q "FURY" runtime/objects/ContentLabHazards.lua
grep -q "THREAD" runtime/objects/ContentLabHazards.lua
grep -q "COUNTER" runtime/objects/ContentLabHazards.lua
grep -q "draw_boss_pressure_aura" runtime/ui/ProceduralEffects.lua
grep -q "buff_remaining" runtime/ui/ProceduralEffects.lua

run_args=(--headless)
if [[ "${LANARTS_SKIP_BUILD:-0}" != "0" ]]; then
    run_args+=(--force)
fi

env SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./run.sh "${run_args[@]}" engine.StartContentApiSmoke "$@"
