#!/bin/bash
# Builds lanarts with CMake and runs it from the runtime directory.
#
#   ./run.sh [options] [lanarts arguments...]
#
# Build configuration (one of; see CMakePresets.json, default: debug):
#   -O, --release      optimised build with debug info
#   --headless         no graphics or sound, for simulations and tests
#   --sanitize         debug build with AddressSanitizer and UBSan
#   -lj, --luajit      release build using LuaJIT
#   --web              Emscripten build (needs emcmake / EMSCRIPTEN_ROOT)
#   --mingw            Windows cross-compile with MinGW-w64
#   --preset NAME      any configure preset
#
# Other options:
#   -b, --build        build only, do not run
#   -f, --force        do not build, run the last successful build
#   --clean            clean before building
#   -v, --verbose      show the full build output
#   -g, --gdb          run under gdb (lldb on macOS)
#   -s, --small        run in an 800x600 window (sets LANARTS_SMALL)
#
# Anything else is passed to lanarts, eg: ./run.sh --headless engine.StartMapGenCheck --seed 1

set -e
cd "$(dirname "$0")"

preset=""
build=1
run=1
clean=0
verbose=0
debugger=0
args=()

choose() {
    if [ -n "$preset" ] && [ "$preset" != "$1" ]; then
        echo "run.sh: conflicting build configurations '$preset' and '$1'" >&2
        exit 1
    fi
    preset=$1
}

while [ $# -gt 0 ]; do
    case "$1" in
        -O|--release|--optimize) choose release ;;
        --headless) choose headless ;;
        --sanitize) choose sanitize ;;
        -lj|--luajit) choose luajit ;;
        --web|--emscripten) choose web ;;
        --mingw) choose mingw ;;
        --preset) shift; choose "$1" ;;
        -b|--build) run=0 ;;
        -f|--force) build=0 ;;
        --clean) clean=1 ;;
        -v|--verbose) verbose=1 ;;
        -g|--gdb) debugger=1 ;;
        -s|--small) export LANARTS_SMALL=1 ;;
        *) args+=("$1") ;;
    esac
    shift
done
preset=${preset:-debug}
build_dir=build/$preset

# Older versions of this script made 'build' a symlink to build_debug/build_release
if [ -L build ]; then
    rm build
fi

if [ $build = 1 ]; then
    configure=(cmake --preset "$preset")
    if [ "$preset" = web ] && [ -z "$EMSCRIPTEN_ROOT" ]; then
        configure=(emcmake cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release)
    fi
    build_cmd=(cmake --build "$build_dir" --parallel)
    if [ $clean = 1 ]; then
        build_cmd+=(--clean-first)
    fi
    if [ $verbose = 1 ]; then
        "${configure[@]}"
        "${build_cmd[@]}"
    else
        # Quiet unless something fails
        for step in configure build_cmd; do
            eval "cmd=(\"\${$step[@]}\")"
            if ! log=$("${cmd[@]}" 2>&1); then
                echo "$log" | tail -60 >&2
                exit 1
            fi
        done
    fi
fi

if [ $run = 0 ] || [ "$preset" = web ] || [ "$preset" = mingw ]; then
    exit 0
fi

cd runtime
binary=../$build_dir/src/lanarts
if [ $debugger = 1 ]; then
    if command -v gdb > /dev/null; then
        exec gdb -silent -x ../debug.gdb --args "$binary" "${args[@]}"
    else
        exec lldb -- "$binary" "${args[@]}"
    fi
fi
exec "$binary" "${args[@]}"
