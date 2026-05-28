set -e

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
./run.sh --headless engine.StartContentApiSmoke "$@"
