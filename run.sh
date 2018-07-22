cd `dirname $0`

# Good practice -- exit completely on any bad exit code:
set -e

###############################################################################
# Helper functions for conditionally coloring text.
###############################################################################

function is_mac() {
    if [ "$(uname)" == "Darwin" ]; then
        return 0 # True!
    else
        return 1 # False!
    fi
}

# Bash function to apply a color to a piece of text.
function colorify() {
    if is_mac ; then
        cat
    else
        local words;
        words=$(cat)
        echo -e "\e[$1m$words\e[0m"
    fi
}

###############################################################################
# Bash function to check for a flag in 'args' and remove it.
# Treats 'args' as one long string.
# Returns true if flag was removed.
###############################################################################

args="$@" # Create a mutable copy of the program arguments
function handle_flag(){
    flag=$1
    local new_args
    local got
    got=1 # False!
    for arg in $args ; do
        if [ $arg = $flag ] ; then
            args="${args/$flag/}"
            got=0 # True!
        else
            new_args="$new_args $arg"
        fi
    done
    args="$new_args"
    return $got # False!
}

##############################################################################
# Eclipse options
#   --eclipse/-e: Create eclipse project files
###############################################################################

# Create eclipse-project-files
if handle_flag "--eclipse" || handle_flag "-e" ; then
    src=$(pwd)
    rm -f CMakeCache.txt
    mkdir ../LanartsEclipse -p
    cd ../LanartsEclipse
    # Eclipse project creation
    cmake -Wno-dev -G"Eclipse CDT4 - Unix Makefiles" $src
    exit
fi

###############################################################################
# Compiling and setting up runtime directory structure
###############################################################################

# Handle environment-variable setting convenience flags
# These are used to communicate with CMake
# Each flag has an optional shortform, use whichever is preferred.

if handle_flag "--mingw" ; then
    export BUILD_MINGW=1
fi
if handle_flag "--headless" ; then
    export BUILD_HEADLESS=1
fi
if handle_flag "--luajit" || handle_flag "-lj" ; then
    export BUILD_LUAJIT=1
fi
if handle_flag "--optimize" || handle_flag "-O" ; then
    export BUILD_OPTIMIZE=1
fi
if handle_flag "--sanitize"  ; then
    export BUILD_SANITIZE=1
fi
if handle_flag "--profile-gen" || handle_flag "--pgen" ; then
    export BUILD_OPTIMIZE=1
    export BUILD_PROF_GEN=1
fi
# Use --pgen, and then this flag, for optimal performance
if handle_flag "--profile-use" || handle_flag "--puse" ; then
    export BUILD_OPTIMIZE=1
    export BUILD_PROF_USE=1
fi
# Pick whether to use debug std data-structures for eg std::vector
if handle_flag "--debug-std" ; then
    export BUILD_FLAGS="$BUILD_FLAGS -D_GLIBCXX_DEBUG"
fi

# Configure amount of cores used
if [[ -e /proc/cpuinfo ]] ; then
    cores=$(grep -c ^processor /proc/cpuinfo)
else
    cores=4 # Guess -- may want to manually edit if above fails.
fi
# Helper for managing build directories:
function rm_if_link(){ [ ! -L "$1" ] || rm -f "$1"; }
function build_lanarts(){
    BUILD_DIR="build_debug"
    if [ $BUILD_OPTIMIZE ] ; then
        BUILD_DIR="build_release"
    fi
    # Specialize build dirs
    if [ $BUILD_HEADLESS ] ; then
        BUILD_DIR="${BUILD_DIR}_headless"
    fi
    if [ $BUILD_LUAJIT ] ; then
        BUILD_DIR="${BUILD_DIR}_luajit"
    fi
    if [ $BUILD_SANITIZE ] ; then
        BUILD_DIR="${BUILD_DIR}_asan"
    fi
    if [ $BUILD_PROF_GEN ] ; then
        BUILD_DIR="${BUILD_DIR}_profgen"
    fi
    if [ $BUILD_PROF_USE ] ; then
        BUILD_DIR="${BUILD_DIR}_profuse"
    fi
    if [ $BUILD_MINGW ] ; then
        BUILD_DIR="${BUILD_DIR}_mingw"
    fi
    rm_if_link build
    if [ -d build ] ; then
        echo "You have a non-symlink build directory. Lanarts has moved to symlinking 'build' to 'build_release' or 'build_debug'. Please rename the build directory to the appropriate one of those." >&2
        exit 1
    fi
    mkdir -p $BUILD_DIR
    ln -s $BUILD_DIR build
    cd $BUILD_DIR
    if [ $BUILD_MINGW ] ; then
        if python -mplatform | grep fedora ; then
            export BUILD_FEDORA_CROSS=1
            mingw32-cmake  -Wno-dev .. | colorify '1;33'
        else
            cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake  -Wno-dev .. | colorify '1;33'
        fi
    else
	cmake -Wno-dev .. | colorify '1;33'
    fi
    if handle_flag "--clean" ; then
        make clean
    fi
    make -j$((cores+1)) lanarts
    cd ../runtime && python2 compile_images.py > compiled/Resources.lua
    cd ..
}

#   --force/-f: Do not build (use last successful compiled binary)
if ! handle_flag "-f" && ! handle_flag "--force" ; then
    if handle_flag "--verbose" || handle_flag "-v" ; then
        build_lanarts
    else
        build_lanarts > /dev/null
    fi
fi

#   --build/-b: Do not run (build only)
if handle_flag "-b" || handle_flag "--build" ; then
    exit
fi

###############################################################################
# Running the game.
###############################################################################

function run_lanarts(){
    cd runtime
    if handle_flag "--gdb" || handle_flag "-g" ; then
        echo "Wrapping in GDB:" | colorify '1;35'
        gdb -silent -ex="break lua_error" -ex="break abort" -ex="catch throw" -ex=r --args ../build/src/lanarts $args
    else
        exec ../build/src/lanarts $args
    fi
    cd ..
}
# TODO add gdb macro with this line:
#print luaL_loadstring(L, "return debug.traceback()") || lua_pcall(L, 0, 1, 0) || printf(lua_tolstring(L, -1, 0))

run_lanarts
