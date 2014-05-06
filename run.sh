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
    cmake -G"Eclipse CDT4 - Unix Makefiles" $src
    exit
fi

###############################################################################
# Compiling and setting up runtime directory structure
###############################################################################

# Handle environment-variable setting convenience flags
# These are used to communicate with CMake
# Each flag has an optional shortform, use whichever is preferred.

if handle_flag "--vanilla-lua" || handle_flag "-vl" ; then
    export BUILD_NO_LUAJIT=1
fi
if handle_flag "--optimize" || handle_flag "-O" ; then
    export BUILD_OPTIMIZE=1
fi
if handle_flag "--sanitize" || handle_flag "-S" ; then
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

function build_lanarts(){
    mkdir -p build
    cd build
    cmake .. | colorify '1;33'
    if handle_flag "--clean" ; then
        make clean
    fi
    make -j$((cores+1))
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

###############################################################################
# Running the game. 
###############################################################################

function run_lanarts(){
    cd runtime
    if handle_flag "--gdb" || handle_flag "-g" ; then
        echo "Wrapping in GDB:" | colorify '1;35'
        gdb -silent -ex=r --args ../build/src/lanarts $args
    else
        ../build/src/lanarts $args
    fi
    cd ..
}

run_lanarts
