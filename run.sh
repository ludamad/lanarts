if [ $(basename $(pwd)) == 'build-scripts' ] ; then cd .. ; fi

args="$@"
function build() {
    buildcmd='./build-scripts/make.sh'
    if [[ "$args" == *"-f"* ]] ; then
        args="${args/-f/}"
        return
    fi
    if [[ "$args" == *"--verbose"* ]] ; then
        args="${args/--verbose/}"
        $buildcmd
    elif [[ "$args" == *"-v"* ]] ; then
        args="${args/-v/}"
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
if [[ "$args" == *"--gdb"* ]] ; then
    args="${args/--gdb/}"    
    gdb --args ../lanarts_build/lanarts/lanarts $args
else
    ../lanarts_build/lanarts/lanarts $args
fi
