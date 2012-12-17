if ! ./make.sh; then
	echo "Build failed, aborting lanarts run."
	exit $?
fi

# Run lanarts

cd ../lanarts
../lanarts_build/src/lanarts/lanarts
