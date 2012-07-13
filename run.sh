if ./make.sh; then
	echo "Build failed, aborting lanarts run."
	exit $?
fi

# Run lanarts

cd ../lanarts
vblank_mode=0 ../lanarts_build/src/lanarts
