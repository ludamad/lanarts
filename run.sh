if [ ! ./make.sh ]; then
	echo "Build failed, aborting lanarts run."
	return -1
fi

# Run lanarts

cd ../lanarts
export vblank_mode=0
../lanarts_build/src/lanarts
