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
grep -q "avg_package_micro_punish_total" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_micro_punish_break_total" runtime/engine/StartBalanceRun.lua
grep -q "avg_dice_rolls_total" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_early_rate" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_timing_score" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_counter_windows" runtime/engine/StartBalanceRun.lua
grep -q "avg_package_rescue_fatal_misses" runtime/engine/StartBalanceRun.lua
grep -q "mana-sapper-ritual-package" runtime/engine/ContentCatalog.lua
grep -q "mana-sapper-ritual-package" web/content_lab/shell.html
grep -q "MICRO" runtime/objects/ContentLabHazards.lua
grep -q "BEAT" runtime/objects/ContentLabHazards.lua
grep -q "FURY" runtime/objects/ContentLabHazards.lua
grep -q "THREAD" runtime/objects/ContentLabHazards.lua
grep -q "COUNTER" runtime/objects/ContentLabHazards.lua
grep -q "draw_boss_pressure_aura" runtime/ui/ProceduralEffects.lua

run_args=(--headless)
if [[ "${LANARTS_SKIP_BUILD:-0}" != "0" ]]; then
    run_args+=(--force)
fi

env SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./run.sh "${run_args[@]}" engine.StartContentApiSmoke "$@"
