# Run a script in the lanarts lua environment
# Note that the lanarts lua API will be loaded, but lanarts game data will not
# Usage lua-run.sh script.lua

if ! ./make.sh; then
	echo "Build failed, aborting lanarts-lua run."
	exit $?
fi

# Run lanarts
gdb --tui --args ../lanarts_build/src/lanarts/lanarts_runner $@
