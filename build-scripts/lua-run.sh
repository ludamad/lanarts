# Run a script in the lanarts lua environment
# Note that the lanarts lua API will be loaded, but lanarts game data will not
# Usage lua-run.sh script.lua

if [ $(basename $(pwd)) == 'build-scripts' ] ; then cd .. ; fi

if ! ./build-scripts/make.sh > /dev/null ; then
	echo "Build failed, aborting lanarts-lua run."
	exit $?
fi

# Run lanarts
../lanarts_build/lanarts/lanarts_runner $@
