if [ $(basename $(pwd)) == 'build-scripts' ] ; then cd .. ; fi
args="$@"
source './build-scripts/util.sh'
function build() {
    buildcmd='./build-scripts/make.sh'
    if handle_flag "-f" ; then return ; fi
    if handle_flag "--verbose" || handle_flag "-v" ; then
        $buildcmd
    else
        $buildcmd > /dev/null
    fi
}

if ! build ; then
    echo "Build failed, aborting lanarts run."
    exit $?
fi

# Run lanarts
if handle_flag "--gdb" || handle_flag "-g" ; then
    echo "Wrapping in GDB:" | colorify '1;35'
    gdb -silent -ex=r --args ../lanarts_build/src/lanarts $args
else
    ../lanarts_build/src/lanarts $args
fi
