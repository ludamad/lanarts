set -e

grep -q "_lanarts_content_package_matrix_json" CMakeLists.txt
grep -q "_lanarts_content_map_matrix_json" CMakeLists.txt
grep -q "_lanarts_content_fight_setup_json" CMakeLists.txt
grep -q "_lanarts_content_presentation_json" CMakeLists.txt
./run.sh --headless engine.StartContentApiSmoke "$@"
