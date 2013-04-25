if ! ./make.sh; then
	echo "Build failed, aborting lanarts run."
	exit $?
fi

# Run lanarts
../lanarts_build/src/lanarts/lanarts
